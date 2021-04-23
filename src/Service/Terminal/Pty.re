open Oni_Core;
open Utility;

module NamedPipe = Exthost.NamedPipe;
module Packet = Exthost.Transport.Packet;

module UniqueId =
  UniqueId.Make({});

module Log = (val Log.withNamespace("Service_Terminal.Pty"));

type t = {
  onData: string => unit,
  write: string => unit,
};

// Setup process
let createProcess = (~setup, ~namedPipeStr, ~command) => {
  let process = {
    open Base.Result.Let_syntax;
    let%bind stdoutPipe = Luv.Pipe.init();
    let%bind stderrPipe = Luv.Pipe.init();

    prerr_endline("--created pipes");
    let buffers = ref([]);

    let on_exit = (_, ~exit_status, ~term_signal as _) => {
      prerr_endline("Pty process exited!");
      Log.infof(m => m("on_exit called with exit_status: %Ld", exit_status));
    };

    let scriptPath = Setup.getNodeScriptPath(~script="pty-helper.js", setup);

    let args = [namedPipeStr, Sys.getcwd(), command];
    let%bind process =
      LuvEx.Process.spawn(
        ~on_exit,
        ~environment=[],
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

let start = (~setup, ~env, ~cwd, ~rows, ~cols, ~cmd, onData) => {
  open Base.Result.Let_syntax;
  let uniqueId = UniqueId.create(~friendlyName="Pty");

  let namedPipe = NamedPipe.create(UniqueId.toString(uniqueId));
  let namedPipeStr = NamedPipe.toString(namedPipe);
  Log.infof(m => m("Starting terminal pipe: %s", namedPipeStr));

  let dispatch = (msg: Exthost.Transport.msg) => {
    switch (msg) {
    | Connected => prerr_endline("--connected")
    | Received({body, _}: Exthost.Transport.Packet.t) =>
      onData(Bytes.to_string(body))
    | msg =>
      prerr_endline("unknown message: " ++ Exthost.Transport.show_msg(msg))
    };
  };

  let%bind server =
    Exthost.Transport.start(~namedPipe=namedPipeStr, ~dispatch);

  let write = str => {
    let packet =
      Packet.create(
        ~bytes=Bytes.of_string(str),
        ~packetType=Packet.Regular,
        ~id=0,
      );
    Exthost.Transport.send(~packet, server);
  };

  let%bind process = createProcess(~setup, ~namedPipeStr, ~command=cmd);

  Ok({onData, write});
};

let write = ({write, _}, data) => {
  write(data);
};

let resize = (~rows, ~cols, _pty) => {
  prerr_endline("TODO: Resize!");
};

let close = pty => ();
