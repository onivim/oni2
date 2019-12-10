/*
 Syntax client
 */

module Core = Oni_Core;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

module ClientLog = (val Core.Log.withNamespace("Oni2.SyntaxClient"));
module ServerLog = (val Core.Log.withNamespace("Oni2.SyntaxServer"));

exception Test(string);

type t = {
  in_channel: Stdlib.in_channel,
  out_channel: Stdlib.out_channel,
  readThread: Thread.t,
};

let write = (client: t, msg: Protocol.ClientToServer.t) => {
  Marshal.to_channel(client.out_channel, msg, []);
  Stdlib.flush(client.out_channel);
};

let start = (~onHighlights, languageInfo, setup) => {
  let (pstdin, stdin) = Unix.pipe();
  let (stdout, pstdout) = Unix.pipe();
  let (stderr, pstderr) = Unix.pipe();

  Unix.set_close_on_exec(pstdin);
  Unix.set_close_on_exec(stdin);
  Unix.set_close_on_exec(pstdout);
  Unix.set_close_on_exec(stdout);
  Unix.set_close_on_exec(pstderr);
  Unix.set_close_on_exec(stderr);

  let env = 
  ["__ONI2_PARENT_PID__="++string_of_int(Unix.getpid()),
  ...Array.to_list(Unix.environment())
  ];


  let pid =
    Unix.create_process_env(
      Sys.executable_name,
      [|Sys.executable_name, "--syntax-highlight-service"|],
      Array.of_list(env),
      pstdin,
      pstdout,
      pstderr,
    );

  let shouldClose = ref(false);

  let in_channel = Unix.in_channel_of_descr(stdout);
  let out_channel = Unix.out_channel_of_descr(stdin);

  Stdlib.set_binary_mode_in(in_channel, true);
  Stdlib.set_binary_mode_out(out_channel, true);

  let waitThread =
    Thread.create(
      () => {
        let (code, _status: Unix.process_status) = Unix.waitpid([], pid);
        ClientLog.error(
          "Syntax process closed with exit code: " ++ string_of_int(code),
        );
      },
      (),
    );

  let readThread =
    Thread.create(
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stdout);
          let result: ServerToClient.t = Marshal.from_channel(in_channel);
          switch (result) {
          | ServerToClient.EchoReply(result) =>
            ClientLog.info("got message from channel: |" ++ result ++ "|")
          | ServerToClient.Log(msg) => ServerLog.info(msg)
          | ServerToClient.TokenUpdate(tokens) =>
            onHighlights(tokens);
            ClientLog.info("Tokens applied");
          };
        }
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
  write(v, message);
};

let notifyBufferLeave = (_v: t, _bufferId: int) => {
  ClientLog.info("TODO - Send Buffer leave.");
};

let notifyThemeChanged = (v: t, theme: TokenTheme.t) => {
  write(v, Protocol.ClientToServer.ThemeChanged(theme));
};

let notifyBufferUpdate =
    (v: t, bufferUpdate: Oni_Core.BufferUpdate.t, lines: array(string)) => {
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate, lines));
};

let notifyVisibilityChanged = (v: t, visibility) =>
  write(v, Protocol.ClientToServer.VisibleRangesChanged(visibility));
