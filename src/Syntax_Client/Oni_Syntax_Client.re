/*
 Syntax client
 */

module Core = Oni_Core;

open Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module ServerToClient = Protocol.ServerToClient;

module LogClient = (val Core.Log.withNamespace("Syntax.client"));
module LogServer = (val Core.Log.withNamespace("Syntax.server"));

type t = {
  in_channel: Stdlib.in_channel,
  out_channel: Stdlib.out_channel,
  readThread: Thread.t,
  //writeThread: Thread.t,
};

let write = (v: t, msg: Protocol.ClientToServer.t) => {
  Marshal.to_channel(v.out_channel, msg, []);
  Stdlib.flush(v.out_channel);
};

exception SyntaxProcessCrashed;

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

  let pid =
    Unix.create_process(
      Sys.executable_name,
      [|Sys.executable_name, "--syntax-highlight-service"|],
      pstdin,
      pstdout,
      pstderr,
    );

  let shouldClose = ref(false);

  let in_channel = Unix.in_channel_of_descr(stdout);
  let out_channel = Unix.out_channel_of_descr(stdin);

  let waitThread =
    Thread.create(
      () => {
        let _ = Unix.waitpid([], pid);
        LogClient.error("SYNTAX PROCESS CRASHED");
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
            LogClient.info("got message from channel: |" ++ result ++ "|")
          | ServerToClient.Log(msg) => LogServer.info(msg)
          | ServerToClient.TokenUpdate(tokens) =>
            LogClient.info(
              "Got " ++ string_of_int(List.length(tokens)) ++ " tokens!",
            );
            onHighlights(tokens);
            LogClient.info("Tokens applied");
          };
        }
      },
      (),
    );

  /*let writeThread = Thread.create(() => {

     let count = ref(0);
     while (!shouldClose^) {
      incr(count);
      print_endline ("Writing!");
      let message: Oni_Syntax.Protocol.ClientToServer.t = Oni_Syntax.Protocol.ClientToServer.Echo("yoyoyo" ++ string_of_int(count^));
      Marshal.to_channel(out_channel, message, []);
      Stdlib.flush(out_channel);
      Unix.sleepf(0.5);

     }
    }, ());*/
  LogClient.info("started syntax client");
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
  LogClient.info("TODO - Send Buffer leave.");
};

let notifyThemeChanged = (v: t, theme: TokenTheme.t) => {
  write(v, Protocol.ClientToServer.ThemeChanged(theme));
};

let notifyBufferUpdate =
    (v: t, bufferUpdate: Oni_Core.BufferUpdate.t, lines: array(string)) => {
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate, lines));
};
