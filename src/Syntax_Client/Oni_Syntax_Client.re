/*
 Syntax client
 */

type t = unit;

let log = (msg) => print_endline ("[Syntax Client] " ++ msg);

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
    
    let _ =
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

     let _readThread = Thread.create(() => {
      while (!shouldClose^) {
        Thread.wait_read(stdout);
         log("Got message");
         let result: Oni_Syntax.Protocol.ServerToClient.t  = Marshal.from_channel(in_channel);
         switch (result) {
         | Oni_Syntax.Protocol.ServerToClient.EchoReply(result) => log("got message from channel: |" ++ result ++ "|");
         | _ => ();
         }
      }
     }, ());

    let _writeThread = Thread.create(() => {

      let count = ref(0);
      while (!shouldClose^) {
       incr(count);
       let message = Oni_Syntax.Protocol.ClientToServer.Echo("yoyoyo" ++ string_of_int(count^));
       Marshal.to_channel(out_channel, message, []);
       Stdlib.flush(out_channel);
       Unix.sleepf(0.5);

      }
     }, ());
 ();
};

let notifyBufferEnter = (_v: t, _bufferId: int, fileType: string, _lines: array(string)) => {
 log("Buffer enter.");
};

let notifyBufferLeave = (_v: t, _bufferId: int) => {
 log("Buffer enter.");
};
