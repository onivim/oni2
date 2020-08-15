open TestFramework;
open Exthost;
open Exthost_TestLib;

module Header = Transport.Packet.Header;
module Packet = Transport.Packet;

module Test = {
  type t = {
    exits: ref(bool),
    messages: ref(list(Transport.msg)),
    transport: Transport.t,
  };

  let uniqueId = ref(0);

  let getNamedPipe = () => {
    let namedPipe =
      NamedPipe.create("transport-test-" ++ (uniqueId^ |> string_of_int))
      |> NamedPipe.toString;
    incr(uniqueId);
    namedPipe;
  };

  let packetFromString = str => {
    let bytes = str |> Bytes.of_string;
    let packet = Transport.Packet.create(~bytes, ~packetType=Regular, ~id=0);
    packet;
  };

  let packetMatchesString = (str, {body, _}: Packet.t) => {
    let packetString = Bytes.to_string(body);
    String.equal(str, packetString);
  };

  let start = scriptPath => {
    let messages = ref([]);
    let dispatch = msg => messages := [msg, ...messages^];

    let namedPipe = getNamedPipe();

    let exits = ref(false);
    let onExit = (_proc, ~exit_status as _, ~term_signal as _) =>
      exits := true;
    let _: Luv.Process.t = Node.spawn(~onExit, [scriptPath, namedPipe]);
    let transport = Transport.start(~namedPipe, ~dispatch) |> Result.get_ok;

    {exits, messages, transport};
  };

  let waitForMessagef = (~name, f, {messages, _} as context) => {
    Waiter.wait(~name, () => messages^ |> List.exists(f));
    context;
  };

  let waitForMessage = (~name, msg, context) => {
    waitForMessagef(~name, m => m == msg, context);
  };

  let closeTransport = ({transport, _} as context) => {
    Transport.close(transport);
    context;
  };

  let send = (packet, {transport, _} as context) => {
    Transport.send(~packet, transport);
    context;
  };

  let waitForExit = ({exits, _} as context) => {
    Waiter.wait(() => exits^);
    context;
  };
};

describe("Transport", ({describe, _}) => {
  describe("process sanity checks", ({test, _}) => {
    test("start node", _ => {
      let exits = ref(false);
      let onExit = (_proc, ~exit_status as _, ~term_signal as _) =>
        exits := true;
      let _ = Node.spawn(~onExit, ["--version"]);

      Waiter.wait(() => exits^ == true);
    });

    test("node process GC", _ => {
      let exits = ref(false);
      let onExit = (_proc, ~exit_status as _, ~term_signal as _) =>
        exits := true;
      let _proc: Luv.Process.t = Node.spawn(~onExit, ["--version"]);
      Waiter.wait(() => exits^ == true);
      // TODO:
      // waitForCollection(~name="proc", proc);
      //Gc.finalise_last(() => collected := true, proc);
      //wait(checkCollected);
    });
  });

  describe("server", ({test, _}) => {
    test("disconnect from server--side", _ => {
      let {transport, _}: Test.t =
        Test.start("test/collateral/node/client.js")
        |> Test.waitForMessage(~name="connect", Transport.Connected)
        |> Test.closeTransport
        |> Test.waitForExit;

      Waiter.waitForCollection(~name="transport", transport);
    });
    test("disconnect from client-side", _ => {
      let {transport, _}: Test.t =
        Test.start("test/collateral/node/immediate-disconnect-client.js")
        |> Test.waitForMessage(~name="connect", Transport.Connected)
        |> Test.waitForExit
        |> Test.waitForMessage(~name="disconnect", Transport.Disconnected);

      Waiter.waitForCollection(~name="transport", transport);
    });
    test("echo", _ => {
      let packet = Test.packetFromString("Hello, world!");

      let {transport, _}: Test.t =
        Test.start("test/collateral/node/echo-client.js")
        |> Test.waitForMessage(~name="connect", Transport.Connected)
        |> Test.send(packet)
        |> Test.waitForMessagef(
             ~name="echo reply",
             fun
             | Transport.Received(packet) =>
               Test.packetMatchesString("Hello, world!", packet)
             | _ => false,
           )
        |> Test.closeTransport
        |> Test.waitForExit;

      Waiter.waitForCollection(~name="transport", transport);
    });
    test("echo: queued message before send", _ => {
      let packet1 = Test.packetFromString("Hello, world1!");
      let packet2 = Test.packetFromString("Hello, world2!");

      let {transport, _}: Test.t =
        Test.start("test/collateral/node/echo-client.js")
        |> Test.send(packet1)
        |> Test.send(packet2)
        |> Test.waitForMessage(~name="connect", Transport.Connected)
        |> Test.waitForMessagef(
             ~name="echo reply 1",
             fun
             | Transport.Received(packet) =>
               Test.packetMatchesString("Hello, world1!", packet)
             | _ => false,
           )
        |> Test.waitForMessagef(
             ~name="echo reply 2",
             fun
             | Transport.Received(packet) =>
               Test.packetMatchesString("Hello, world2!", packet)
             | _ => false,
           )
        |> Test.closeTransport
        |> Test.waitForExit;

      Waiter.waitForCollection(~name="transport", transport);
    });
  });

  describe("client connect", ({test, _}) => {
    test("simple client/server", ({expect, _}) => {
      let namedPipe = Test.getNamedPipe();

      let serverConnected = ref(false);
      let serverGotMessage = ref(false);
      let serverDispatch =
        fun
        | Transport.Connected => serverConnected := true
        | Transport.Received(packet) => {
            expect.equal(
              Test.packetMatchesString("Hello from client!", packet),
              true,
            );
            serverGotMessage := true;
          }
        | _ => ();

      let server =
        Transport.start(~namedPipe, ~dispatch=serverDispatch) |> Result.get_ok;

      let clientConnected = ref(false);
      let clientGotMessage = ref(false);
      let clientDispatch =
        fun
        | Transport.Connected => clientConnected := true
        | Transport.Received(packet) => {
            expect.equal(
              Test.packetMatchesString("Hello from server!", packet),
              true,
            );
            clientGotMessage := true;
          }
        | _ => ();

      let client =
        Transport.connect(~namedPipe, ~dispatch=clientDispatch)
        |> Result.get_ok;

      Waiter.wait(~name="Wait for client / server connection", () => {
        serverConnected^ && clientConnected^
      });

      Transport.send(
        ~packet=Test.packetFromString("Hello from client!"),
        client,
      );

      Waiter.wait(~name="Wait for server to receive message", () => {
        serverGotMessage^
      });

      Transport.send(
        ~packet=Test.packetFromString("Hello from server!"),
        server,
      );

      Waiter.wait(~name="Wait for client to receive message", () => {
        clientGotMessage^
      });

      Transport.close(client);
      Transport.close(server);
    })
  });
});
