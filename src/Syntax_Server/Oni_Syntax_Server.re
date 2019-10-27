/*
 Syntax client
 */

open Oni_Syntax;

let write = (msg: Protocol.ServerToClient.t) => {
  Marshal.to_channel(Stdlib.stdout, msg, []);
  Stdlib.flush(Stdlib.stdout);
};

let start = () => {
  while (true) {
    let msg: Oni_Syntax.Protocol.ClientToServer.t =
      Marshal.from_channel(Stdlib.stdin);
    switch (msg) {
    | exception _ => write(Protocol.ServerToClient.Log("Got an exception"))
    | Echo(m) => write(Protocol.ServerToClient.EchoReply(m))
    | BufferEnter(id, fileType, lines) =>
      write(
        Protocol.ServerToClient.Log(
          Printf.sprintf(
            "Got buffer enter for id: %d of filetype %s with %d lines",
            id,
            fileType,
            Array.length(lines),
          ),
        ),
      )
    | BufferUpdate(bufferUpdate) =>
      write(
        Protocol.ServerToClient.Log(
          Printf.sprintf(
            "Got buffer update for id: %d with %d lines",
            bufferUpdate.id,
            Array.length(bufferUpdate.lines),
          ),
        ),
      )
    | _ => ()
    };
  };
};
