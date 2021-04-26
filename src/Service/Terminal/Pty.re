open Oni_Core;
open Utility;

module NamedPipe = Exthost.NamedPipe;
module Packet = Exthost.Transport.Packet;

module UniqueId =
  UniqueId.Make({});

module Log = (val Log.withNamespace("Service_Terminal.Pty"));

module Protocol = {
  module Outgoing = {
    let userInput: string => Packet.t =
      input => {
        Packet.create(
          ~packetType=Packet.Regular,
          ~ack=0,
          ~id=0,
          Bytes.of_string(input),
        );
      };

    let resize: (~rows: int, ~cols: int) => Packet.t =
      (~rows, ~cols) => {
        let json =
          Printf.sprintf(
            {|
        {
          "rows": %d,
          "cols": %d
        }
      |},
            rows,
            cols,
          );
        Packet.create(
          ~ack=1,
          ~packetType=Packet.Regular,
          ~id=0,
          Bytes.of_string(json),
        );
      };

    let close =
      Packet.create(~ack=2, ~packetType=Packet.Regular, ~id=0, Bytes.empty);
  };
};

type t = {
  onData: string => unit,
  write: Packet.t => unit,
  process: Luv.Process.t,
};

module Internal = {
  // Setup process
  let createProcess =
      (~cwd, ~env, ~rows, ~cols, ~setup, ~namedPipeStr, ~command, ~arguments) => {
    let process = {
      open Base.Result.Let_syntax;

      let on_exit = (_, ~exit_status, ~term_signal as _) => {
        Log.infof(m =>
          m("on_exit called with exit_status: %Ld", exit_status)
        );
      };

      let scriptPath =
        Setup.getNodeScriptPath(~script="pty-helper.js", setup);

      let args =
        [
          namedPipeStr,
          cwd,
          command,
          string_of_int(rows),
          string_of_int(cols),
        ]
        @ arguments;
      let%bind process =
        LuvEx.Process.spawn(
          ~on_exit,
          ~environment=env,
          ~redirect=[],
          setup.nodePath,
          [setup.nodePath, scriptPath, ...args],
        );

      Log.info("Successfully created process.");

      Ok(process);
    };

    process |> Result.map_error(Luv.Error.strerror);
  };
};

let start =
    (
      ~setup,
      ~env,
      ~cwd,
      ~rows,
      ~cols,
      ~cmd,
      ~arguments,
      ~onData,
      ~onPidChanged,
      ~onTitleChanged,
      ~onExit,
    ) => {
  open Base.Result.Let_syntax;
  let uniqueId = UniqueId.create(~friendlyName="Pty");

  let namedPipe = NamedPipe.create(UniqueId.toString(uniqueId));
  let namedPipeStr = NamedPipe.toString(namedPipe);
  Log.infof(m => m("Starting terminal pipe: %s", namedPipeStr));

  let dispatch = (msg: Exthost.Transport.msg) => {
    switch (msg) {
    | Connected => Log.info("Connected.")
    | Received({body, header}: Exthost.Transport.Packet.t) =>
      switch (header.ack) {
      | 0 => onData(Bytes.to_string(body))
      //  TODO: Parse exit code
      | 1 => onExit(~exitCode=0)
      | 2 =>
        body
        |> Bytes.to_string
        |> int_of_string_opt
        |> Option.iter(onPidChanged)
      | 3 =>
        let process = body |> Bytes.to_string;
        if (Sys.win32 && process == "xterm-color") {
          cmd |> onTitleChanged;
        } else {
          process |> onTitleChanged;
        };
      | unknownType =>
        Log.errorf(m => m("Unknown message type (ack): %d", unknownType))
      }
    | msg =>
      Log.errorf(m =>
        m("unknown message: %s", Exthost.Transport.show_msg(msg))
      )
    };
  };

  let%bind server =
    Exthost.Transport.start(~namedPipe=namedPipeStr, ~dispatch);

  let write = packet => {
    Exthost.Transport.send(~packet, server);
  };

  let%bind process =
    Internal.createProcess(
      ~setup,
      ~rows,
      ~cols,
      ~env,
      ~namedPipeStr,
      ~command=cmd,
      ~cwd,
      ~arguments,
    );

  Ok({onData, write, process});
};

let write = ({write, _}, data) => {
  let packet = Protocol.Outgoing.userInput(data);
  write(packet);
};

let resize = (~rows, ~cols, {write, _}) => {
  Log.info("Trying to resize");
  let packet = Protocol.Outgoing.resize(~rows, ~cols);
  write(packet);
};

let close = ({write, _}) => {
  Log.info("Trying to close");
  let packet = Protocol.Outgoing.close;
  write(packet);
};
