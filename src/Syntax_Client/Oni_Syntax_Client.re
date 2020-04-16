/*
 Syntax client
 */

open Oni_Core;
open Utility;

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

type t = {
  transport: Transport.t,
  process: Luv.Process.t,
  nextId: ref(int),
};

let write = ({transport, nextId, _}: t, msg: Protocol.ClientToServer.t) => {
  incr(nextId);
  let id = nextId^;

  let bytes = Marshal.to_bytes(msg, []);
  let packet = Transport.Packet.create(~packetType=Regular, ~id, ~bytes);
  Transport.send(~packet, transport);
};

let start =
    (
      ~onConnected=() => (),
      ~onClose=_ => (),
      ~scheduler,
      ~onHighlights,
      ~onHealthCheckResult,
      languageInfo,
      setup,
    ) => {
  //let dispatch = fun
  //| _ => ();

  let namedPipe = NamedPipe.create("test") |> NamedPipe.toString;
  //let transport = Transport.start(~namedPipe, ~dispatch);

  let parentPid = Unix.getpid() |> string_of_int;

  let filterOutLogFile = List.filter(env => fst(env) != "ONI2_LOG_FILE");

  // TODO: Proper mapping
  let environment =
    Luv.Env.environ()
    |> Result.map(filterOutLogFile)
    |> Result.map(items =>
         [(EnvironmentVariables.parentPid, parentPid), ...items]
       )
    |> Result.get_ok;

  let executableName = "Oni2_editor" ++ (Sys.win32 ? ".exe" : "");
  let executablePath = Revery.Environment.executingDirectory ++ executableName;

  ClientLog.debugf(m =>
    m("Starting executable: %s and parentPid: %s", executablePath, parentPid)
  );

  let on_exit = (proc, ~exit_status, ~term_signal) => {
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
  };
  let process =
    Luv.Process.spawn(
      ~on_exit,
      ~environment,
      ~windows_hide=true,
      ~windows_hide_console=true,
      ~windows_hide_gui=true,
      executablePath,
      [executablePath, "--syntax-highlight-service"],
    )
    |> Result.get_ok;

  // TODO: Remove scheduler
  let scheduler = cb => Scheduler.run(cb, scheduler);

  let handleMessage = msg =>
    switch (msg) {
    | ServerToClient.Initialized => scheduler(onConnected)
    | ServerToClient.EchoReply(result) =>
      scheduler(() =>
        ClientLog.tracef(m => m("got message from channel: |%s|", result))
      )
    | ServerToClient.Log(msg) => scheduler(() => ServerLog.trace(msg))
    | ServerToClient.Closing => scheduler(() => ServerLog.debug("Closing"))
    | ServerToClient.HealthCheckPass(res) =>
      scheduler(() => onHealthCheckResult(res))
    | ServerToClient.TokenUpdate(tokens) =>
      scheduler(() => {
        onHighlights(tokens);
        ClientLog.trace("Tokens applied");
      })
    };

  let handlePacket = bytes => {
    let msg: ServerToClient.t =
      Marshal.from_bytes(bytes, Bytes.length(bytes));
    handleMessage(msg);
  };

  let dispatch =
    fun
    | Transport.Connected => ClientLog.info("Connected to server")
    | Transport.Error(msg) => ClientLog.errorf(m => m("Error: %s", msg))
    | Transport.Disconnected => ClientLog.info("Disconnected")
    | Transport.Received({body, _}) => handlePacket(body);

  let transport = Transport.start(~namedPipe, ~dispatch) |> Result.get_ok;

  let ret = {transport, process, nextId: ref(0)};

  write(ret, Protocol.ClientToServer.Initialize(languageInfo, setup));
  ret;
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
  write(v, Protocol.ClientToServer.ThemeChanged(theme));
};

let notifyConfigurationChanged = (v: t, configuration: Configuration.t) => {
  write(v, Protocol.ClientToServer.ConfigurationChanged(configuration));
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
