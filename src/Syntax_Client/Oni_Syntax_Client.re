/*
 Syntax client
 */

open Oni_Syntax;

type t = {
  in_channel: Stdlib.in_channel,
  out_channel: Stdlib.out_channel,
  readThread: Thread.t,
  //writeThread: Thread.t,
};

let log = msg => print_endline("[Syntax Client] " ++ msg);

let start = () => {
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
        print_endline("syntax process exited!");
      },
      (),
    );

  let readThread =
    Thread.create(
      () => {
        while (! shouldClose^) {
          Thread.wait_read(stdout);
          let result: Oni_Syntax.Protocol.ServerToClient.t =
            Marshal.from_channel(in_channel);
          switch (result) {
          | Oni_Syntax.Protocol.ServerToClient.EchoReply(result) =>
            log("got message from channel: |" ++ result ++ "|")
          | Oni_Syntax.Protocol.ServerToClient.Log(msg) =>
            log("log message: " ++ msg)
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
  log("started syntax client");
  {in_channel, out_channel, readThread /*writeThread*/};
};

let write = (v: t, msg: Protocol.ClientToServer.t) => {
  Marshal.to_channel(v.out_channel, msg, []);
  Stdlib.flush(v.out_channel);
};

let notifyBufferEnter =
    (v: t, bufferId: int, fileType: string, lines: array(string)) => {
  let message: Oni_Syntax.Protocol.ClientToServer.t =
    Oni_Syntax.Protocol.ClientToServer.BufferEnter(bufferId, fileType, lines);
  write(v, message);
};

let notifyBufferLeave = (_v: t, _bufferId: int) => {
  log("Buffer leave.");
};

let notifyBufferUpdate = (v: t, bufferUpdate: Oni_Core.BufferUpdate.t) => {
  write(v, Protocol.ClientToServer.BufferUpdate(bufferUpdate));
};
