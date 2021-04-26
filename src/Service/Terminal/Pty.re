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
};

module Internal = {
  // Setup process
  let createProcess = (~env, ~setup, ~namedPipeStr, ~command, ~arguments) => {
    let process = {
      open Base.Result.Let_syntax;
      let%bind stdoutPipe = Luv.Pipe.init();
      let%bind stderrPipe = Luv.Pipe.init();

      prerr_endline("--created pipes");
      let buffers = ref([]);

      let on_exit = (_, ~exit_status, ~term_signal as _) => {
        prerr_endline("Pty process exited!");
        Log.infof(m =>
          m("on_exit called with exit_status: %Ld", exit_status)
        );
      };

      let scriptPath =
        Setup.getNodeScriptPath(~script="pty-helper.js", setup);

      let args = [namedPipeStr, Sys.getcwd(), command] @ arguments;
      let%bind process =
        LuvEx.Process.spawn(
          ~on_exit,
          ~environment=env,
          ~redirect=[
            Luv.Process.to_parent_pipe(
              ~fd=Luv.Process.stdout,
              ~parent_pipe=stdoutPipe,
              (),
            ),
            Luv.Process.to_parent_pipe(
              ~fd=Luv.Process.stderr,
              ~parent_pipe=stderrPipe,
              (),
            ),
          ],
          setup.nodePath,
          [setup.nodePath, scriptPath, ...args],
        );
      prerr_endline("--created process");

      let () =
        Luv.Stream.read_start(
          stdoutPipe,
          fun
          | Error(`EOF) => {
              Log.info("Got EOF on stdout");
              Luv.Handle.close(stdoutPipe, ignore);
            }
          | Error(msg) => Log.error(Luv.Error.strerror(msg))
          | Ok(buffer) =>
            print_endline("STDOUT: " ++ Luv.Buffer.to_string(buffer)),
        );

      let () =
        Luv.Stream.read_start(
          stderrPipe,
          fun
          | Error(`EOF) => {
              Log.info("Got EOF on stderr");
              Luv.Handle.close(stderrPipe, ignore);
            }
          | Error(msg) => Log.error(Luv.Error.strerror(msg))
          | Ok(buffer) =>
            print_endline("STDERR: " ++ Luv.Buffer.to_string(buffer)),
        );
      Ok(process);
    };

    let ret: result(Luv.Process.t, string) =
      process |> Result.map_error(Luv.Error.strerror);
    ret;
  };
};

let start =
    (~setup, ~env, ~cwd, ~rows, ~cols, ~cmd, ~arguments, ~onData, ~onExit) => {
  open Base.Result.Let_syntax;
  let uniqueId = UniqueId.create(~friendlyName="Pty");

  let namedPipe = NamedPipe.create(UniqueId.toString(uniqueId));
  let namedPipeStr = NamedPipe.toString(namedPipe);
  Log.infof(m => m("Starting terminal pipe: %s", namedPipeStr));

  let dispatch = (msg: Exthost.Transport.msg) => {
    switch (msg) {
    | Connected => prerr_endline("--connected")
    | Received({body, header}: Exthost.Transport.Packet.t) =>
      switch (header.ack) {
      | 0 => onData(Bytes.to_string(body))
      //  TODO: Parse exit code
      | 1 => onExit(~exitCode=0)
      | unknownType =>
        prerr_endline(
          "Unknown message type (ack): " ++ string_of_int(unknownType),
        )
      }
    | msg =>
      prerr_endline("unknown message: " ++ Exthost.Transport.show_msg(msg))
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
      ~env,
      ~namedPipeStr,
      ~command=cmd,
      ~arguments,
    );

  Ok({onData, write});
};

let write = ({write, _}, data) => {
  let packet = Protocol.Outgoing.userInput(data);
  write(packet);
};

let resize = (~rows, ~cols, {write, _}) => {
  prerr_endline("Trying to resize");
  let packet = Protocol.Outgoing.resize(~rows, ~cols);
  write(packet);
};

let close = ({write, _}) => {
  prerr_endline("Trying to close");
  let packet = Protocol.Outgoing.close;
  write(packet);
};
