/*
 Syntax client
 */

module Core = Oni_Core;
module Ext = Oni_Extensions;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

module ClientLog = (val Core.Log.withNamespace("Oni2.SyntaxClient"));
module ServerLog = (val Core.Log.withNamespace("Oni2.SyntaxServer"));

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
      ~onConnected=Core.Utility.noop,
      ~onClose=Core.Utility.noop1,
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
  let camomilePath = Core.Setup.(setup.camomilePath);

  // Remove ONI2_LOG_FILE from environment of syntax server
  let envList =
    Unix.environment()
    |> Array.to_list
    |> List.filter(str =>
         !Core.Utility.StringUtil.contains("ONI2_LOG_FILE", str)
       );

  let env = [
    Core.EnvironmentVariables.parentPid ++ "=" ++ parentPid,
    Core.EnvironmentVariables.camomilePath ++ "=" ++ camomilePath,
    ...envList,
  ];

  let executableName =
    Revery.Environment.executingDirectory
    ++ "Oni2_editor"
    ++ (Sys.win32 ? ".exe" : "");

  ClientLog.infof(m =>
    m(
      "Starting executable: %s with camomilePath: %s and parentPid: %s",
      executableName,
      camomilePath,
      parentPid,
    )
  );

  let pid =
    Unix.create_process_env(
      executableName,
      [|executableName, "--syntax-highlight-service"|],
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

  let scheduler = cb => Core.Scheduler.run(cb, scheduler);

  let safeClose = channel =>
    switch (Unix.close(channel)) {
    // We may get a BADF if this is called too soon after opening a process
    | exception (Unix.Unix_error(_)) => ()
    | _ => ()
    };

  let _waitThread =
    Core.ThreadHelper.create(
      ~name="SyntaxThread.wait",
      () => {
        let (_pid, status: Unix.process_status) = Unix.waitpid([], pid);
        let code =
          switch (status) {
          | Unix.WEXITED(0) =>
            ClientLog.info("Syntax process exited safely.");
            0;
          | Unix.WEXITED(code) =>
            ClientLog.error(
              "Syntax process exited with code: " ++ string_of_int(code),
            );
            code;
          | Unix.WSIGNALED(signal) =>
            ClientLog.error(
              "Syntax process stopped with signal: " ++ string_of_int(signal),
            );
            signal;
          | Unix.WSTOPPED(signal) =>
            ClientLog.error(
              "Syntax process stopped with signal: " ++ string_of_int(signal),
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
    Core.ThreadHelper.create(
      ~name="SyntaxThread.read",
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stdout);
          let result: ServerToClient.t = Marshal.from_channel(in_channel);
          switch (result) {
          | ServerToClient.Initialized => scheduler(onConnected)
          | ServerToClient.EchoReply(result) =>
            scheduler(() =>
              ClientLog.info("got message from channel: |" ++ result ++ "|")
            )
          | ServerToClient.Log(msg) => scheduler(() => ServerLog.info(msg))
          | ServerToClient.Closing =>
            scheduler(() => ServerLog.info("Closing"))
          | ServerToClient.HealthCheckPass(res) =>
            scheduler(() => onHealthCheckResult(res))
          | ServerToClient.TokenUpdate(tokens) =>
            scheduler(() => {
              onHighlights(tokens);
              ClientLog.info("Tokens applied");
            })
          };
        };

        safeClose(stdout);
      },
      (),
    );

  let _readStderr =
    Core.ThreadHelper.create(
      ~name="SyntaxThread.stderr",
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stderr);
          switch (input_line(err_channel)) {
          | exception End_of_file => shouldClose := true
          | v => scheduler(() => ServerLog.info(v))
          };
        };
        safeClose(stderr);
        scheduler(() => ServerLog.info("stderr thread done!"));
      },
      (),
    );
  ClientLog.info("started syntax client");
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
  ClientLog.info("Sending bufferUpdate notification...");
  write(v, message);
};

let notifyBufferLeave = (_v: t, _bufferId: int) => {
  ClientLog.info("TODO - Send Buffer leave.");
};

let notifyThemeChanged = (v: t, theme: TokenTheme.t) => {
  write(v, Protocol.ClientToServer.ThemeChanged(theme));
};

let notifyConfigurationChanged =
    (v: t, configuration: Oni_Core.Configuration.t) => {
  write(v, Protocol.ClientToServer.ConfigurationChanged(configuration));
};

let healthCheck = (v: t) => {
  write(v, Protocol.ClientToServer.RunHealthCheck);
};

let notifyBufferUpdate =
    (
      v: t,
      bufferUpdate: Oni_Core.BufferUpdate.t,
      lines: array(string),
      scope,
    ) => {
  ClientLog.info("Sending bufferUpdate notification...");
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate, lines, scope));
};

let notifyVisibilityChanged = (v: t, visibility) => {
  ClientLog.info("Sending visibleRangesChanged notification...");
  write(v, Protocol.ClientToServer.VisibleRangesChanged(visibility));
};

let close = (syntaxClient: t) => {
  ClientLog.info("Sending close request...");
  write(syntaxClient, Protocol.ClientToServer.Close);
};
