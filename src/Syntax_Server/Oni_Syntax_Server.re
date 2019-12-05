/*
 Syntax client
 */

open Oni_Syntax;

module Protocol = Oni_Syntax.Protocol;
module ClientToServer = Protocol.ClientToServer;

type message =
  | Log(string)
  | Message(ClientToServer.t)
  | Exception;

let start = () => {
  // A list of pending messages for us to handle
  let messageQueue: ref(list(message)) = ref([]);
  // Mutex to guard accessing the queue from multiple threads
  let messageMutex = Mutex.create();

  let queue = msg => {
    Mutex.lock(messageMutex);
    messageQueue := [msg, ...messageQueue^];
    Mutex.unlock(messageMutex);
  };

  let flush = () => {
    Mutex.lock(messageMutex);
    let result = messageQueue^;
    messageQueue := [];
    Mutex.unlock(messageMutex);
    List.rev(result);
  };

  let isRunning = ref(true);

  let _runThread: Thread.t =
    Thread.create(
      () => {
        let write = (msg: Protocol.ServerToClient.t) => {
          Marshal.to_channel(Stdlib.stdout, msg, []);
          Stdlib.flush(Stdlib.stdout);
        };

        let log = msg => write(Protocol.ServerToClient.Log(msg));

        let state = ref(State.empty);
        let map = f => state := f(state^);

        let handleProtocol =
          fun
          | ClientToServer.Echo(m) =>
            write(Protocol.ServerToClient.EchoReply(m))
          | v => log("Unhandled message: " ++ ClientToServer.show(v));

        let handleMessage =
          fun
          | Log(msg) => log(msg)
          | Exception => log("Exception encountered!")
          | Message(protocol) => handleProtocol(protocol);

        while (true) {
          log("Syntax Server - tick");

          // Get pending messages and handle them
          flush() |> List.iter(handleMessage);

          Unix.sleepf(1.0);
          // Wait for incoming messages
          // Handle messages
          // If any work, flush work
        };
      },
      (),
    );

  let _readThread: Thread.t =
    Thread.create(
      () => {
        while (isRunning^) {
          queue(Log("Waiting for message from client..."));
          let msg: Oni_Syntax.Protocol.ClientToServer.t =
            Marshal.from_channel(Stdlib.stdin);
          queue(Log("Got message from client!"));
          switch (msg) {
          | exception _ => queue(Exception)
          | msg => queue(Message(msg))
          };
        }
      },
      (),
    );

  Thread.join(_runThread);
};
/*log("Got language info!!")
        map(State.setLanguageInfo(languageInfo))
      | Echo(m) => write(Protocol.ServerToClient.EchoReply(m))
      | ThemeChanged(theme) =>
        log("Got new theme!")
        map(State.updateTheme(theme))
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
  };*/
