/*
 Syntax Server
 */

module Protocol = Oni_Syntax.Protocol;
module ClientToServer = Protocol.ClientToServer;

type message =
  | Log(string)
  | Message(ClientToServer.t)
  | Exception(string);

let start = (~healthCheck) => {
  Stdlib.set_binary_mode_out(Stdlib.stdout, true);
  Stdlib.set_binary_mode_in(Stdlib.stdin, true);

  // A list of pending messages for us to handle
  let messageQueue: ref(list(message)) = ref([]);
  // Mutex to guard accessing the queue from multiple threads
  let messageMutex = Mutex.create();
  // And a semaphore for signaling when we have a new message
  let messageCondition = Condition.create();

  let outputMutex = Mutex.create();

  let queue = msg => {
    Mutex.lock(messageMutex);
    messageQueue := [msg, ...messageQueue^];
    Condition.signal(messageCondition);
    Mutex.unlock(messageMutex);
  };

  let write = (msg: Protocol.ServerToClient.t) => {
    Mutex.lock(outputMutex);
    Marshal.to_channel(Stdlib.stdout, msg, []);
    Stdlib.flush(Stdlib.stdout);
    Mutex.unlock(outputMutex);
  };

  let log = msg => write(Protocol.ServerToClient.Log(msg));

  let buffer = Buffer.create(0);

  // Route Core.Log logging to be sent
  // to main process.
  let formatter =
    Format.make_formatter(
      Buffer.add_substring(buffer),
      () => {
        log(Buffer.contents(buffer));
        Buffer.clear(buffer);
      },
    );
  Logs.format_reporter(~app=formatter, ~dst=formatter, ())
  |> Logs.set_reporter;

  let hasPendingMessage = () =>
    switch (messageQueue^) {
    | [] => false
    | _ => true
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
        log(
          "Starting up server. Parent PID is: " ++ string_of_int(parentPid),
        );

        write(Protocol.ServerToClient.Initialized);

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
            | RunHealthCheck => {
                let res = healthCheck();
                write(Protocol.ServerToClient.HealthCheckPass(res == 0));
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
            | ConfigurationChanged(config) => {
                map(State.updateConfiguration(config));
                let treeSitterEnabled =
                  Oni_Core.Configuration.getValue(
                    c => c.experimentalTreeSitter,
                    config,
                  );
                log(
                  "got new config - treesitter enabled:"
                  ++ (treeSitterEnabled ? "true" : "false"),
                );
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
            | Close => {
                write(Protocol.ServerToClient.Closing);
                exit(0);
              }
            | SimulateMessageException => failwith("Exception!")
            | v => log("Unhandled message: " ++ ClientToServer.show(v))
          );

        let handleMessage =
          fun
          | Log(msg) => log(msg)
          | Exception(msg) => log("Exception encountered: " ++ msg)
          | Message(protocol) => handleProtocol(protocol);

        while (isRunning^) {
          try(
            {
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
            }
          ) {
          | ex =>
            queue(Exception(Printexc.to_string(ex)));
            exit(2);
          };
        };
      },
      (),
    );

  let _readThread: Thread.t =
    Thread.create(
      () => {
        while (isRunning^) {
          try({
            let msg: Oni_Syntax.Protocol.ClientToServer.t =
              Marshal.from_channel(Stdlib.stdin);

            switch (msg) {
            | SimulateReadException => failwith("Exception")
            | msg => queue(Message(msg))
            };
          }) {
          | ex =>
            queue(Exception(Printexc.to_string(ex)));
            exit(2);
          };
        };
        ();
      },
      (),
    );

  log("Testing...");

  let waitForPidWindows = pid =>
    try({
      let (_exitCode, _status) = Thread.wait_pid(pid);
      ();
    }) {
    // If the PID doesn't exist, Thread.wait_pid will throw
    | _ex => ()
    };

  let waitForPidPosix = pid => {
    while (true) {
      Unix.sleepf(5.0);
      try(Unix.kill(pid, 0)) {
      // If we couldn't send signal 0, the process is dead:
      // https://stackoverflow.com/questions/3043978/how-to-check-if-a-process-id-pid-exists
      | _ex => exit(0)
      };
    };
  };

  let waitForPid = Sys.win32 ? waitForPidWindows : waitForPidPosix;

  let _parentProcessWatcherThread: Thread.t =
    Thread.create(() => {waitForPid(parentPid)}, ());

  // On Windows, we have to wait on the parent to close...
  // If we don't do this, the app will never close.
  if (Sys.win32) {
    let () = Thread.join(_parentProcessWatcherThread);
    ();
  } else {
    // On Linux / OSX, the syntax process will close when
    // the parent closes.
    let () = Thread.join(_readThread);
    ();
  };

  isRunning := false;
  Stdlib.close_out_noerr(Stdlib.stdout);
  Stdlib.close_in_noerr(Stdlib.stdin);
  ();
};
