/*
 Syntax client
 */


let start = () => {
  //let _readThread = Thread.create(() => {
    while (true) {
      let msg: Oni_Syntax.Protocol.ClientToServer.t = 
      Marshal.from_channel(Stdlib.stdin);
      switch (msg) {
      | exception _ => 
       Marshal.to_channel(Stdlib.stdout, Oni_Syntax.Protocol.ServerToClient.Log("Got an exception"), []);
      | Echo(m) => Marshal.to_channel(Stdlib.stdout, Oni_Syntax.Protocol.ServerToClient.EchoReply(m), []);
      | BufferEnter(id, fileType, lines) =>
        Marshal.to_channel(Stdlib.stdout, Oni_Syntax.Protocol.ServerToClient.Log(
          Printf.sprintf("Got buffer enter for id: %d of filetype %s with %d lines", id, fileType, Array.length(lines))
        ), []);
      | _ => ();
      }
      //Marshal.to_channel(Stdlib.stdout, "ECHO: " ++ msg, []);
      Stdlib.flush(Stdlib.stdout);
      };
    //}
  //}, ());

  //let count = ref(0);
  //let _ = exit(0);
};
