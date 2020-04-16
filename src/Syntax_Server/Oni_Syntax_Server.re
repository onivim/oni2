/*
 Syntax Server
 */

module Protocol = Oni_Syntax.Protocol;
module ClientToServer = Protocol.ClientToServer;

module Transport = Exthost.Transport;

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

  let parentPid = Unix.getenv("__ONI2_PARENT_PID__") |> int_of_string;
  let namedPipe = Unix.getenv("__ONI2_NAMED_PIPE__");

  log("Starting up server. Parent PID is: " ++ string_of_int(parentPid));

  let state = ref(State.empty);
  let map = f => state := f(state^);

  // Route Core.Log logging to be sent
  // to main process.
  /*let formatter =
      Format.make_formatter(
        Buffer.add_substring(buffer),
        () => {
          log(Buffer.contents(buffer));
          Buffer.clear(buffer);
        },
      );
    Logs.format_reporter(~app=formatter, ~dst=formatter, ())
    |> Logs.set_reporter;*/

  let handleProtocol =
    ClientToServer.(
      fun
      | Echo(m) => {
          write(Protocol.ServerToClient.EchoReply(m));
          log("handled echo");
        }
      | Initialize(languageInfo, setup) => {
          map(State.initialize(~log, languageInfo, setup));
          write(Protocol.ServerToClient.Initialized);
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
      | v => log("Unhandled message: " ++ ClientToServer.show(v))
    );

  let handleMessage =
    fun
    | Log(msg) => log(msg)
    | Exception(msg) => log("Exception encountered: " ++ msg)
    | Message(protocol) => handleProtocol(protocol);

  let handlePacket = ({body, _}: Transport.Packet.t) => {
    let msg: Protocol.ClientToServer.t = Marshal.from_bytes(body, 0);

    handleMessage(Message(msg));
  };

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
