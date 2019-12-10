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
  Stdlib.set_binary_mode_out(Stdlib.stdout, true);
  Stdlib.set_binary_mode_in(Stdlib.stdin, true);

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

  let parentPid = Unix.getenv("__ONI2_PARENT_PID__") |> int_of_string;

  let _runThread: Thread.t =
    Thread.create(
      () => {
        let write = (msg: Protocol.ServerToClient.t) => {
          Marshal.to_channel(Stdlib.stdout, msg, []);
          Stdlib.flush(Stdlib.stdout);
        };

        let log = msg => write(Protocol.ServerToClient.Log(msg));

        log(
          "Starting up server. Parent PID is: " ++ string_of_int(parentPid),
        );

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
            | BufferUpdate(bufferUpdate, lines, scope) => {
                map(State.bufferUpdate(~bufferUpdate, ~lines, ~scope));
                log(
                  Printf.sprintf(
                    "Received buffer update - %d | %d lines",
                    bufferUpdate.id,
                    Array.length(lines),
                  ),
                );
              }
            | VisibleRangesChanged(visibilityUpdate) => {
                map(State.updateVisibility(visibilityUpdate));
              }
            | v => log("Unhandled message: " ++ ClientToServer.show(v))
          );

        let handleMessage =
          fun
          | Log(msg) => log(msg)
          | Exception => log("Exception encountered!")
          | Message(protocol) => handleProtocol(protocol);

        while (isRunning^) {
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
          let msg: Oni_Syntax.Protocol.ClientToServer.t =
            Marshal.from_channel(Stdlib.stdin);
          switch (msg) {
          | exception _ => queue(Exception)
          | msg => queue(Message(msg))
          };
        }
      },
      (),
    );

  // On Windows, we have to wait on the parent to close...
  // If we don't do this, the app will never close.
  if (Sys.win32) {
    let (_exitCode, _status) = Thread.wait_pid(parentPid);
    ();
  } else {
    // On Linux / OSX, the syntax process will close when
    // the parent closes.
    let () = Thread.join(_readThread);
    ();
  };
};
