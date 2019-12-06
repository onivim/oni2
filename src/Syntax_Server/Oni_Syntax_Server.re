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
  // And a semaphore for signaling when we have a new message
  let messageCondition = Condition.create();

  let queue = msg => {
    Mutex.lock(messageMutex);
    messageQueue := [msg, ...messageQueue^];
    Condition.signal(messageCondition);
    Mutex.unlock(messageMutex);
  };

  let hasPendingMessage = () =>
    switch (messageQueue^) {
    | [] => false
    | [hd, ..._] => true
    };

  let flush = () => {
    let result = messageQueue^;
    messageQueue := [];
    result;
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
          ClientToServer.(
            fun
            | Echo(m) => {
                write(Protocol.ServerToClient.EchoReply(m));
                log("handled echo");
              }
            | Initialize(languageInfo, setup) => {
                map(State.initialize(~log, languageInfo, setup));
                log("Initialized!");
              }
            | BufferEnter(id, filetype) => {
                log(
                  Printf.sprintf(
                    "Buffer enter - id: %d filetype: %s",
                    id,
                    filetype,
                  ),
                );
                map(State.bufferEnter(id));
              }
            | ThemeChanged(theme) => {
                map(State.updateTheme(theme));
                log("handled theme changed");
              }
            | BufferUpdate(bufferUpdate, lines) => {
                map(State.bufferUpdate(~bufferUpdate, ~lines));
                log(
                  Printf.sprintf(
                    "Received buffer update - %d | %d lines",
                    bufferUpdate.id,
                    Array.length(lines),
                  ),
                );
              }
            | v => log("Unhandled message: " ++ ClientToServer.show(v))
          );

        let handleMessage =
          fun
          | Log(msg) => log(msg)
          | Exception => log("Exception encountered!")
          | Message(protocol) => handleProtocol(protocol);

        while (true) {
          log("Waiting for incoming message...");

          // Wait for pending incoming messages
          Mutex.lock(messageMutex);
          while (!hasPendingMessage() && !State.anyPendingWork(state^)) {
            Condition.wait(messageCondition, messageMutex);
          };

          // Once we have them, let's track and run them
          let messages = flush();
          Mutex.unlock(messageMutex);

          // Handle messages
          messages
          // Messages are queued in inverse order, so we need to fix that...
          |> List.rev
          |> List.iter(handleMessage);

          // TODO: Do this in a loop
          // If the messages incurred work, do it!
          if (State.anyPendingWork(state^)) {
            log("Running unit of work...");
            map(State.doPendingWork);

            log("Unit of work completed.");
          } else {
            log("No pending work.");
          };

          log("Sending token updates...");
          let tokenUpdates = State.getTokenUpdates(state^);
          write(Protocol.ServerToClient.TokenUpdate(tokenUpdates));
          log("Token updates sent.");
          map(State.clearTokenUpdates);
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
