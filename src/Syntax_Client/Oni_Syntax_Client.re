/*
 Syntax client
 */

open Oni_Core;
open Oni_Core_Kernel;
open Oni_Core_Utility;

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
  in_channel: Stdlib.in_channel,
  out_channel: Stdlib.out_channel,
  readThread: Thread.t,
};

let write = (client: t, msg: Protocol.ClientToServer.t) => {
  Marshal.to_channel(client.out_channel, msg, []);
  Stdlib.flush(client.out_channel);
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
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();
  let (stderr, pstderr) = Unix.pipe();

  Unix.set_close_on_exec(pstdin);
  Unix.set_close_on_exec(stdin);
  Unix.set_close_on_exec(pstdout);
  Unix.set_close_on_exec(stdout);
  Unix.set_close_on_exec(pstderr);
  Unix.set_close_on_exec(stderr);

  let parentPid = Unix.getpid() |> string_of_int;
  let camomilePath = Setup.(setup.camomilePath);

  // Remove ONI2_LOG_FILE from environment of syntax server
  let envList =
    Unix.environment()
    |> Array.to_list
    |> List.filter(str => !StringEx.contains("ONI2_LOG_FILE", str));

  let env = [
    EnvironmentVariables.parentPid ++ "=" ++ parentPid,
    EnvironmentVariables.camomilePath ++ "=" ++ camomilePath,
    ...envList,
  ];

  let executableName = "Oni2_editor" ++ (Sys.win32 ? ".exe" : "");
  let executablePath = Revery.Environment.executingDirectory ++ executableName;

  ClientLog.debugf(m =>
    m(
      "Starting executable: %s with camomilePath: %s and parentPid: %s",
      executablePath,
      camomilePath,
      parentPid,
    )
  );

  let pid =
    Unix.create_process_env(
      executablePath,
      [|executablePath, "--syntax-highlight-service"|],
      Array.of_list(env),
      pstdin,
      pstdout,
      pstderr,
    );

  Unix.close(pstdout);
  Unix.close(pstdin);
  Unix.close(pstderr);

  let shouldClose = ref(false);

  let in_channel = Unix.in_channel_of_descr(stdout);
  let err_channel = Unix.in_channel_of_descr(stderr);
  let out_channel = Unix.out_channel_of_descr(stdin);

  Stdlib.set_binary_mode_in(in_channel, true);
  Stdlib.set_binary_mode_out(out_channel, true);

  let scheduler = cb => Scheduler.run(cb, scheduler);

  let safeClose = channel =>
    switch (Unix.close(channel)) {
    // We may get a BADF if this is called too soon after opening a process
    | exception (Unix.Unix_error(_)) => ()
    | _ => ()
    };

  let _waitThread =
    ThreadHelper.create(
      ~name="SyntaxThread.wait",
      () => {
        let (_pid, status: Unix.process_status) = Unix.waitpid([], pid);
        let code =
          switch (status) {
          | Unix.WEXITED(0) =>
            ClientLog.debug("Syntax process exited safely.");
            0;
          | Unix.WEXITED(code) =>
            ClientLog.errorf(m =>
              m("Syntax process exited with code: %i", code)
            );
            code;
          | Unix.WSIGNALED(signal) =>
            ClientLog.errorf(m =>
              m("Syntax process stopped with signal: %i", signal)
            );
            signal;
          | Unix.WSTOPPED(signal) =>
            ClientLog.errorf(m =>
              m("Syntax process stopped with signal: %i", signal)
            );
            signal;
          };
        shouldClose := true;
        safeClose(stdin);
        scheduler(() => onClose(code));
      },
      (),
    );

  let readThread =
    ThreadHelper.create(
      ~name="SyntaxThread.read",
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stdout);

          let result: ServerToClient.t = Marshal.from_channel(in_channel);
          switch (result) {
          | ServerToClient.Initialized => scheduler(onConnected)

          | ServerToClient.EchoReply(result) =>
            scheduler(() =>
              ClientLog.debugf(m =>
                m("got message from channel: |%s|", result)
              )
            )

          | ServerToClient.Log(msg) => scheduler(() => ServerLog.debug(msg))

          | ServerToClient.Closing =>
            scheduler(() => ServerLog.debug("Closing"))

          | ServerToClient.HealthCheckPass(res) =>
            scheduler(() => onHealthCheckResult(res))

          | ServerToClient.TokenUpdate(tokens) =>
            scheduler(() => {
              onHighlights(tokens);
              ClientLog.debug("Tokens applied");
            })
          };
        };

        safeClose(stdout);
      },
      (),
    );

  let _readStderr =
    ThreadHelper.create(
      ~name="SyntaxThread.stderr",
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stderr);
          switch (input_line(err_channel)) {
          | exception End_of_file => shouldClose := true
          | msg => scheduler(() => ServerLog.debug(msg))
          };
        };
        safeClose(stderr);
        scheduler(() => ServerLog.debug("stderr thread done!"));
      },
      (),
    );
  ClientLog.debug("started syntax client");
  let syntaxClient = {in_channel, out_channel, readThread};
  write(
    syntaxClient,
    Protocol.ClientToServer.Initialize(languageInfo, setup),
  );
  syntaxClient;
};

let notifyBufferEnter = (v: t, bufferId: int, fileType: string) => {
  let message: Oni_Syntax.Protocol.ClientToServer.t =
    Oni_Syntax.Protocol.ClientToServer.BufferEnter(bufferId, fileType);
  ClientLog.debug("Sending bufferUpdate notification...");
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
  ClientLog.debug("Sending bufferUpdate notification...");
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate, lines, scope));
};

let notifyVisibilityChanged = (v: t, visibility) => {
  ClientLog.debug("Sending visibleRangesChanged notification...");
  write(v, Protocol.ClientToServer.VisibleRangesChanged(visibility));
};

let close = (syntaxClient: t) => {
  ClientLog.debug("Sending close request...");
  write(syntaxClient, Protocol.ClientToServer.Close);
};
