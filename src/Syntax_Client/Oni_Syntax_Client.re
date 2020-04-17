/*
 Syntax client
 */

open Oni_Core;

module Transport = Exthost.Transport;
module NamedPipe = Exthost.NamedPipe;
module Packet = Exthost.Transport.Packet;

module Ext = Oni_Extensions;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

module ClientLog = (val Log.withNamespace("Oni2.Syntax.Client"));
module ServerLog = (val Log.withNamespace("Oni2.Syntax.Server"));

type connectedCallback = unit => unit;
type closeCallback = int => unit;
type highlightsCallback = list(Protocol.TokenUpdate.t) => unit;

module Defaults = {
  let executableName = "Oni2_editor" ++ (Sys.win32 ? ".exe" : "");
  let executablePath = Revery.Environment.executingDirectory ++ executableName;
};

type t = {
  transport: Transport.t,
  process: Luv.Process.t,
  nextId: ref(int),
};

let writeTransport =
    (~id=0, transport: Transport.t, msg: Protocol.ClientToServer.t) => {
  let bytes = Marshal.to_bytes(msg, []);
  let packet = Transport.Packet.create(~packetType=Regular, ~id, ~bytes);
  Transport.send(~packet, transport);
};

let write = ({transport, nextId, _}: t, msg: Protocol.ClientToServer.t) => {
  incr(nextId);
  let id = nextId^;
  writeTransport(~id, transport, msg);
};

let getEnvironment = (~namedPipe, ~parentPid) => {
  let filterOutLogFile = List.filter(env => fst(env) != "ONI2_LOG_FILE");

  Luv.Env.environ()
  |> Result.map(filterOutLogFile)
  |> Result.map(items =>
       [
         (EnvironmentVariables.namedPipe, namedPipe),
         (EnvironmentVariables.parentPid, parentPid),
         ...items,
       ]
     );
};

let startProcess = (~executablePath, ~namedPipe, ~parentPid, ~onClose) => {
  getEnvironment(~namedPipe, ~parentPid)
  |> Utility.ResultEx.flatMap(environment => {
       ClientLog.debugf(m =>
         m(
           "Starting executable: %s and parentPid: %s",
           executablePath,
           parentPid,
         )
       );

       let on_exit = (_proc, ~exit_status, ~term_signal) => {
         let exitCode = exit_status |> Int64.to_int;
         if (exitCode == 0) {
           ClientLog.debug("Syntax process exited safely.");
         } else {
           ClientLog.errorf(m =>
             m(
               "Syntax process exited with code: %d and signal: %d",
               exitCode,
               term_signal,
             )
           );
         };
         onClose(exitCode);
       };

       Luv.Process.spawn(
         ~on_exit,
         ~environment,
         ~windows_hide=true,
         ~windows_hide_console=true,
         ~windows_hide_gui=true,
         executablePath,
         [executablePath, "--syntax-highlight-service"],
       );
     })
  |> Result.map_error(Luv.Error.strerror);
};

let start =
    (
      ~parentPid=?,
      ~executablePath=Defaults.executablePath,
      ~onConnected=() => (),
      ~onClose=_ => (),
      ~onHighlights,
      ~onHealthCheckResult,
      languageInfo,
      setup,
    ) => {
  let parentPid =
    switch (parentPid) {
    | None => Unix.getpid() |> string_of_int
    | Some(pid) => pid
    };

  let name = Printf.sprintf("syntax-client-%s", parentPid);
  let namedPipe = name |> NamedPipe.create |> NamedPipe.toString;

  let handleMessage = msg =>
    switch (msg) {
    | ServerToClient.Initialized =>
        ClientLog.info("Initialized");
      onConnected()
    | ServerToClient.EchoReply(result) =>
      ClientLog.tracef(m => m("got message from channel: |%s|", result))
    | ServerToClient.Log(msg) => ServerLog.trace(msg)
    | ServerToClient.Closing => ServerLog.debug("Closing")
    | ServerToClient.HealthCheckPass(res) => onHealthCheckResult(res)
    | ServerToClient.TokenUpdate(tokens) =>
      ClientLog.info("Received token update");
      onHighlights(tokens);
      ClientLog.trace("Tokens applied");
    };

  let handlePacket = bytes => {
    let msg: ServerToClient.t = Marshal.from_bytes(bytes, 0);
    handleMessage(msg);
  };

  let _transport = ref(None);

  let dispatch =
    fun
    | Transport.Connected => {
        prerr_endline("!!!!! CONNECTED");
        ClientLog.info("Connected to server");
        _transport^
        |> Option.iter(t =>
             writeTransport(
               ~id=0,
               t,
               Protocol.ClientToServer.Initialize(languageInfo, setup),
             )
           );
      }
    | Transport.Error(msg) => ClientLog.errorf(m => m("Error: %s", msg))
    | Transport.Disconnected => ClientLog.info("Disconnected")
    | Transport.Received({body, _}) => handlePacket(body);

  Transport.start(~namedPipe, ~dispatch)
  |> Utility.ResultEx.tap(transport => _transport := Some(transport))
  |> Utility.ResultEx.flatMap(transport => {
       startProcess(~executablePath, ~parentPid, ~namedPipe, ~onClose)
       |> Result.map(process => {transport, process, nextId: ref(0)})
     });
};

let notifyBufferEnter = (v: t, bufferId: int, fileType: string) => {
  let message: Oni_Syntax.Protocol.ClientToServer.t =
    Oni_Syntax.Protocol.ClientToServer.BufferEnter(bufferId, fileType);
  ClientLog.trace("Sending bufferUpdate notification...");
  write(v, message);
};

let notifyBufferLeave = (_v: t, _bufferId: int) => {
  ClientLog.warn("TODO - Send Buffer leave.");
};

let notifyThemeChanged = (v: t, theme: TokenTheme.t) => {
  ClientLog.info("Notifying theme changed.");
  write(v, Protocol.ClientToServer.ThemeChanged(theme));
};

let notifyConfigurationChanged = (v: t, configuration: Configuration.t) => {
  ClientLog.info("Notifying configuration changed.");
  let useTreeSitter =
    configuration |> Configuration.getValue(c => c.experimentalTreeSitter);
  write(v, Protocol.ClientToServer.UseTreeSitter(useTreeSitter));
};

let healthCheck = (v: t) => {
  write(v, Protocol.ClientToServer.RunHealthCheck);
};

let notifyBufferUpdate =
    (v: t, bufferUpdate: BufferUpdate.t, lines: array(string), scope) => {
  ClientLog.trace("Sending bufferUpdate notification...");
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate, lines, scope));
};

let notifyVisibilityChanged = (v: t, visibility) => {
  ClientLog.trace("Sending visibleRangesChanged notification...");
  write(v, Protocol.ClientToServer.VisibleRangesChanged(visibility));
};

let close = (syntaxClient: t) => {
  ClientLog.debug("Sending close request...");
  write(syntaxClient, Protocol.ClientToServer.Close);
};

module Testing = {
  let simulateReadException = ({transport, _}: t) => {
    let id = 1;
    let bytes = Bytes.make(128, 'a');
    let packet = Transport.Packet.create(~packetType=Regular, ~id, ~bytes);
    ClientLog.trace("Simulating a bad packet...");
    Transport.send(~packet, transport);
  };

  let simulateMessageException = (v: t) => {
    ClientLog.trace("Sending simulateMessageException notification...");
    write(v, Protocol.ClientToServer.SimulateMessageException);
  };
};
