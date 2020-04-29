type reply = unit;

module Protocol = Exthost_Protocol;
module Extension = Exthost_Extension;

type t = {
  client: Protocol.t,
  lastRequestId: ref(int),
};

module Log = (val Timber.Log.withNamespace("Client"));

let start =
    (
      ~initialConfiguration=Configuration.empty,
      ~namedPipe,
      ~initData: Extension.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      (),
    ) => {
  let protocolClient: ref(option(Protocol.t)) = ref(None);
  let lastRequestId = ref(0);
  let send = message =>
    switch (protocolClient^) {
    | None => ()
    | Some(protocol) =>
      Log.info(
        "Sending message: " ++ Protocol.Message.Outgoing.show(message),
      );
      Protocol.send(~message, protocol);
    };

  let dispatch = msg => {
    Protocol.Message.(
      switch (msg) {
      | Incoming.Ready =>
        Log.info("Ready");

        incr(lastRequestId);
        send(Outgoing.Initialize({requestId: lastRequestId^, initData}));
        handler(Ready) |> ignore;

      | Incoming.Initialized =>
        Log.info("Initialized");

        let rpcId =
          "ExtHostConfiguration" |> Handlers.stringToId |> Option.get;

        incr(lastRequestId);
        send(
          Outgoing.RequestJSONArgs({
            requestId: lastRequestId^,
            rpcId,
            method: "$initializeConfiguration",
            args:
              initialConfiguration
              |> Configuration.to_yojson
              |> (json => `List([json])),
            usesCancellationToken: false,
          }),
        );
        handler(Initialized) |> ignore;

        incr(lastRequestId);
        let rpcId = "ExtHostWorkspace" |> Handlers.stringToId |> Option.get;
        send(
          Outgoing.RequestJSONArgs({
            requestId: lastRequestId^,
            rpcId,
            method: "$initializeWorkspace",
            args: `List([]),
            usesCancellationToken: false,
          }),
        );

      | Incoming.ReplyError({payload, _}) =>
        switch (payload) {
        | Message(str) => onError(str)
        | Empty => onError("Unknown / Empty")
        }
      | Incoming.RequestJSONArgs({requestId, rpcId, method, args, _}) =>
        let req = Handlers.handle(rpcId, method, args);
        switch (req) {
        | Ok(msg) =>
          handler(msg) |> ignore; // TODO: Hook up to reply!
          send(Outgoing.ReplyOKEmpty({requestId: requestId}));
        | Error(msg) => onError(msg)
        };
      | _ => ()
      }
    );
  };

  let protocol = Protocol.start(~namedPipe, ~dispatch, ~onError);

  protocol
  |> Result.iter(pc => {
       Log.info("Got protocol client.");
       protocolClient := Some(pc);
     });

  protocol |> Result.map(protocol => {{lastRequestId, client: protocol}});
};

let notify =
    (~rpcName: string, ~method: string, ~args, {lastRequestId, client}: t) => {
  open Protocol.Message;
  let maybeId = Handlers.stringToId(rpcName);
  maybeId
  |> Option.iter(rpcId => {
       incr(lastRequestId);
       let requestId = lastRequestId^;
       Protocol.send(
         ~message=
           Outgoing.RequestJSONArgs({
             rpcId,
             requestId,
             method,
             args,
             usesCancellationToken: false,
           }),
         client,
       );
     });
};

let terminate = ({client, _}) => Protocol.send(~message=Terminate, client);

let close = ({client, _}) => Protocol.close(client);
