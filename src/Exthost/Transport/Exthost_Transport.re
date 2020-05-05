module Log = (val Timber.Log.withNamespace("ExtHost.Transport"));

module ByteWriter = ByteWriter;
module Packet = Packet;

[@deriving show]
type msg =
  | Connected
  | Received([@opaque] Packet.t)
  | Error(string)
  | Disconnected;

type t = {
  maybeServer: ref(option(Luv.Pipe.t)),
  maybeClient: ref(option(Luv.Pipe.t)),
  // [queueMessages] keeps track of messages that we tried
  // to send prior to connection. Once we connect, we should
  // try sending again.
  queuedMessages: ref(list(Packet.t)),
  dispatch: msg => unit,
};

let readBuffer:
  (Packet.Parser.t, Luv.Buffer.t) =>
  (Packet.Parser.t, result(list(Packet.t), string)) =
  (parser, buffer: Luv.Buffer.t) => {
    let byteLen = Luv.Buffer.size(buffer);
    Log.tracef(m => m("Read %d bytes", byteLen));

    try({
      let (newParser, packets) = Packet.Parser.parse(buffer, parser);
      (newParser, Ok(packets));
    }) {
    | exn => (Packet.Parser.initial, Error(Printexc.to_string(exn)))
    };
  };

let handleError = (~dispatch, msg, err) => {
  let msg = Printf.sprintf("%s: %s\n", msg, Luv.Error.strerror(err));
  dispatch(Error(msg));
};

let sendCore = (~dispatch, ~packet, client) => {
  let bytes = Packet.toBytes(packet);
  let byteLen = bytes |> Bytes.length;
  Log.tracef(m => m("Sending %d bytes", byteLen));
  let buffer = Luv.Buffer.from_bytes(bytes);
  Luv.Stream.write(
    client,
    [buffer],
    (err, count) => {
      Log.tracef(m => m("Wrote %d bytes", count));
      err |> Result.iter_error(handleError(~dispatch, "Stream.write"));
    },
  );
};

let flushQueuedMessages = (~dispatch, queuedMessages, client) => {
  let packets = queuedMessages^ |> List.rev;
  queuedMessages := [];

  let send = packet => sendCore(~packet, client);

  packets |> List.iter(send(~dispatch));
};

let read = (~dispatch, clientPipe) => {
  let parser = ref(Packet.Parser.initial);

  let handleClosed = () => {
    Log.info("Connection closed.");
    dispatch(Disconnected);
    Luv.Handle.close(clientPipe, ignore);
  };

  let handleError = handleError(~dispatch);

  Luv.Stream.read_start(
    clientPipe,
    fun
    | Error(`EOF) => handleClosed()
    | Error(msg) => handleError("read_start", msg)
    | Ok(buffer) => {
        let (newParser, packetResult) = readBuffer(parser^, buffer);
        parser := newParser;
        switch (packetResult) {
        | Ok(packets) =>
          List.iter(packet => dispatch(Received(packet)), packets)
        | Error(msg) => dispatch(Error(msg))
        };
      },
  );
};

let clearQueuedMessages = queuedMessages => {
  Log.warn("Clearing queued messages due to failure.");
  queuedMessages := [];
};

let start = (~namedPipe: string, ~dispatch: msg => unit) => {
  let handleError = handleError(~dispatch);
  let maybeClient = ref(None);
  let queuedMessages = ref([]);

  // Listen for an incoming connection...
  let listen = serverPipe => {
    Log.infof(m => m("Listening on pipe: %s\n", namedPipe));
    Luv.Pipe.bind(serverPipe, namedPipe) |> ignore;
    Luv.Stream.listen(
      serverPipe,
      listenResult => {
        // Create a pipe for the client
        let clientPipeResult =
          listenResult
          |> (
            r => {
              Log.info("Trying to create client pipe...");
              Stdlib.Result.bind(r, _ => Luv.Pipe.init());
            }
          )
          |> (
            r => {
              Stdlib.Result.bind(r, pipe => {
                Luv.Stream.accept(~server=serverPipe, ~client=pipe)
                |> Result.map(_ => pipe)
              });
            }
          );

        switch (clientPipeResult) {
        | Ok(pipe) =>
          Log.info("Established connection.");
          maybeClient := Some(pipe);
          flushQueuedMessages(~dispatch, queuedMessages, pipe);
          dispatch(Connected);
          read(~dispatch, pipe);
        | Error(err) =>
          clearQueuedMessages(queuedMessages);
          handleError("listen", err);
        };
      },
    );
  };

  let serverPipeResult =
    Luv.Pipe.init() |> Result.map_error(Luv.Error.strerror);

  serverPipeResult |> Result.iter(listen);

  serverPipeResult
  |> Result.map(server =>
       {
         dispatch,
         queuedMessages,
         maybeServer: ref(Some(server)),
         maybeClient,
       }
     );
};

let send = (~packet, {maybeClient, queuedMessages, dispatch, _}) =>
  switch (maybeClient^) {
  | None => queuedMessages := [packet, ...queuedMessages^]
  | Some(client) => sendCore(~dispatch, ~packet, client)
  };

let connect = (~namedPipe: string, ~dispatch: msg => unit) => {
  let clientPipeResult =
    Luv.Pipe.init() |> Result.map_error(Luv.Error.strerror);

  let client = clientPipeResult |> Result.get_ok;

  let queuedMessages = ref([]);

  Luv.Pipe.connect(
    client,
    namedPipe,
    fun
    | Error(msg) => {
        clearQueuedMessages(queuedMessages);
        dispatch(Error("Connect error: " ++ Luv.Error.strerror(msg)));
      }
    | Ok () => {
        dispatch(Connected);
        read(~dispatch, client);
      },
  );

  Ok({
    maybeServer: ref(None),
    maybeClient: ref(Some(client)),
    queuedMessages,
    dispatch,
  });
};

let close = ({maybeServer, maybeClient, queuedMessages, _}) => {
  queuedMessages := [];
  let logResult = (~msg, res) => {
    switch (res) {
    | Ok () => Log.infof(m => m("%s: success", msg))
    | Error(err) =>
      Log.errorf(m => m("ERROR %s: %s", msg, Luv.Error.strerror(err)))
    };
  };

  let logCallback = (~msg, ()) => Log.info(msg);

  maybeClient^
  |> Option.iter(client => {
       Luv.Stream.shutdown(client, logResult(~msg="Client stream shutdown"));
       Luv.Handle.close(client, logCallback(~msg="Client handle close"));
     });

  maybeServer^
  |> Option.iter(server => {
       Luv.Stream.shutdown(server, logResult(~msg="Server shutdown"));
       Luv.Handle.close(server, logCallback(~msg="Server handle close"));
     });

  maybeClient := None;
  maybeServer := None;
};
