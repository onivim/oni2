/*
 Syntax Server
 */

module Protocol = Oni_Syntax.Protocol;
module ClientToServer = Protocol.ClientToServer;

module Transport = Exthost.Transport;

module Constants = {
  let checkPidInterval = 5000; /* ms */
};

type message =
  | Log(string)
  | Message(ClientToServer.t)
  | Exception(string);

let start = (~healthCheck) => {
  let transport = ref(None);

  let write = (msg: Protocol.ServerToClient.t) => {
    let bytes = Marshal.to_bytes(msg, []);
    let packet = Transport.Packet.create(~bytes, ~packetType=Regular, ~id=0);

    transport^ |> Option.iter(Transport.send(~packet));
  };

  let log = msg => write(Protocol.ServerToClient.Log(msg));
  let logError = exn =>
    write(
      Protocol.ServerToClient.Log(
        exn |> Printexc.to_string |> (str => "ERROR: " ++ str),
      ),
    );

  let parentPid = Unix.getenv("__ONI2_PARENT_PID__") |> int_of_string;
  let namedPipe = Unix.getenv("__ONI2_NAMED_PIPE__");

  log("Starting up server. Parent PID is: " ++ string_of_int(parentPid));

  let state = ref(State.empty);
  let timer: Luv.Timer.t = Luv.Timer.init() |> Result.get_ok;

  let _stopWork = () => Luv.Timer.stop(timer);
  let map = f => state := f(state^);

  let doWork = () =>
    try(
      {
        if (State.anyPendingWork(state^)) {
          map(State.doPendingWork);
        } else {
          let _: result(unit, Luv.Error.t) = _stopWork();
          ();
        };

        let tokenUpdates = State.getTokenUpdates(state^);
        write(Protocol.ServerToClient.TokenUpdate(tokenUpdates));
        map(State.clearTokenUpdates);
      }
    ) {
    | ex =>
      logError(ex);
      exit(2);
    };

  let startWork = () => {
    Luv.Timer.start(~repeat=1, timer, 0, () => {doWork()}) |> Result.get_ok;
  };

  let restartTimer = () => {
    ignore(Luv.Timer.again(timer): result(unit, Luv.Error.t));
  };

  let updateAndRestartTimer = f => {
    state := f(state^);
    restartTimer();
  };

  startWork();

  let handleProtocol =
    ClientToServer.(
      fun
      | Echo(m) => {
          write(Protocol.ServerToClient.EchoReply(m));
        }
      | Initialize(languageInfo, setup) => {
          updateAndRestartTimer(State.initialize(~log, languageInfo, setup));
          write(Protocol.ServerToClient.Initialized);
          log("Initialized!");
        }
      | RunHealthCheck => {
          let res = healthCheck();
          write(Protocol.ServerToClient.HealthCheckPass(res == 0));
        }
      | BufferStartHighlighting({bufferId, filetype, _}) => {
          log(
            Printf.sprintf(
              "Buffer enter - id: %d filetype: %s",
              bufferId,
              filetype,
            ),
          );
          updateAndRestartTimer(State.bufferEnter(~bufferId, ~filetype));
        }
      | UseTreeSitter(useTreeSitter) => {
          updateAndRestartTimer(State.setUseTreeSitter(useTreeSitter));
          log(
            "got new config - treesitter enabled:"
            ++ string_of_bool(useTreeSitter),
          );
        }
      | ThemeChanged(theme) => {
          updateAndRestartTimer(State.updateTheme(theme));
          log("handled theme changed");
        }
      | BufferUpdate(bufferUpdate) => {
          let delta = bufferUpdate.isFull ? "(FULL)" : "(DELTA)";
          log(
            Printf.sprintf(
              "Received buffer update - %d | %d lines %s",
              bufferUpdate.id,
              Array.length(bufferUpdate.lines),
              delta,
            ),
          );
          switch (State.bufferUpdate(~bufferUpdate, state^)) {
          | Ok(newState) =>
            state := newState;
            log("Buffer update successfully applied.");
          | Error(msg) => log("Buffer update failed: " ++ msg)
          };

          restartTimer();
        }
      | BufferVisibilityChanged({bufferId, ranges}) => {
          updateAndRestartTimer(
            State.updateBufferVisibility(~bufferId, ~ranges),
          );
        }
      | Close => {
          write(Protocol.ServerToClient.Closing);
          exit(0);
        }
      | SimulateMessageException => failwith("Simulated Exception!")
      | v => log("Unhandled message: " ++ ClientToServer.show(v))
    );

  let handleMessage =
    fun
    | Log(msg) => log(msg)
    | Exception(msg) => log("Exception encountered: " ++ msg)
    | Message(protocol) => handleProtocol(protocol);

  let handlePacket = ({body, _}: Transport.Packet.t) => {
    let msg: Protocol.ClientToServer.t = Marshal.from_bytes(body, 0);

    try(handleMessage(Message(msg))) {
    | exn =>
      logError(exn);
      exit(2);
    };
  };
  let checkParentPid = pid => {
    Luv.Process.kill_pid(~pid, 0)
    |> Oni_Core.Utility.ResultEx.tap(() => log("Parent still active."))
    |> Result.iter_error(_err => {
         // If we couldn't send signal 0, the process is dead:
         // https://stackoverflow.com/questions/3043978/how-to-check-if-a-process-id-pid-exists
         exit(
           3,
         )
       });
  };

  let processWatcherTimer: Luv.Timer.t = Luv.Timer.init() |> Result.get_ok;
  // Start Process watcher
  Luv.Timer.start(
    ~repeat=Constants.checkPidInterval,
    processWatcherTimer,
    Constants.checkPidInterval,
    () => {
    checkParentPid(parentPid)
  })
  |> Result.get_ok;

  let dispatch =
    fun
    | Transport.Connected => log("Connected!")
    | Transport.Received(packet) => handlePacket(packet)
    | _ => ();

  let transportResult = Exthost.Transport.connect(~namedPipe, ~dispatch);

  transport := transportResult |> Result.to_option;

  let _: bool = Luv.Loop.run();
  ();
};
