type reply = unit;
module Protocol = Exthost_Protocol;
module Extension = Exthost_Extension;

type t = {
  client: Protocol.t,
  lastRequestId: ref(int),
  requestIdToReply: Hashtbl.t(int, Lwt.u(Yojson.Safe.json)),
  initPromise: Lwt.t(unit),
};

module Log = (val Timber.Log.withNamespace("Exthost.Client"));

module Testing = {
  let getPendingRequestCount = ({requestIdToReply, _}) => {
    requestIdToReply |> Hashtbl.length;
  };
};

let start =
    (
      ~initialConfiguration=Configuration.empty,
      ~initialWorkspace=WorkspaceData.fromPath(Sys.getcwd()),
      ~namedPipe,
      ~initData: Extension.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      (),
    ) => {
  let (initPromise, initResolver) = Lwt.task();
  let protocolClient: ref(option(Protocol.t)) = ref(None);
  let lastRequestId = ref(0);
  let requestIdToReply = Hashtbl.create(128);
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
      | Incoming.Connected => Log.info("Connected")
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
            args:
              `List([
                initialWorkspace
                |> Oni_Core.Json.Encode.encode_value(WorkspaceData.encode),
              ]),
            usesCancellationToken: false,
          }),
        );

        Lwt.wakeup(initResolver, ());
      | Incoming.ReplyError({payload, _}) =>
        switch (payload) {
        | Message(str) => onError(str)
        | Empty => onError("Unknown / Empty")
        }
      | Incoming.RequestJSONArgs({requestId, rpcId, method, args, _}) =>
        Log.tracef(m =>
          m("RequestJSONArgs: %d %d %s", requestId, rpcId, method)
        );
        let req = Handlers.handle(rpcId, method, args);
        switch (req) {
        | Ok(msg) =>
          handler(msg) |> ignore; // TODO: Hook up to reply!
          send(Outgoing.ReplyOKEmpty({requestId: requestId}));
        | Error(msg) => onError(msg)
        };
      | Incoming.ReplyOk({requestId, payload}) =>
        Hashtbl.find_opt(requestIdToReply, requestId)
        |> Option.iter(resolver => {
             switch (payload) {
             | Json(json) => Lwt.wakeup(resolver, json)
             | _ =>
               Log.warnf(m =>
                 m("Unhandled payload type for requestId: %d", requestId)
               )
             }
           })
      | Incoming.Acknowledged({requestId}) =>
        Log.tracef(m => m("Received ack: %d", requestId))
      | _ =>
        Log.warn(
          "Unhandled message: " ++ Protocol.Message.Incoming.show(msg),
        )
      }
    );
  };

  let protocol = Protocol.start(~namedPipe, ~dispatch, ~onError);

  protocol
  |> Result.iter(pc => {
       Log.info("Got protocol client.");
       protocolClient := Some(pc);
     });

  protocol
  |> Result.map(protocol => {
       {lastRequestId, client: protocol, requestIdToReply, initPromise}
     });
};

let notify =
    (
      ~usesCancellationToken=false,
      ~rpcName: string,
      ~method: string,
      ~args,
      {lastRequestId, client, initPromise, _}: t,
    ) => {
  Lwt.on_success(
    initPromise,
    () => {
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
                 usesCancellationToken,
               }),
             client,
           );
         });
    },
  );
};

let request =
    (
      ~usesCancellationToken=false,
      ~rpcName: string,
      ~method: string,
      ~args,
      ~decoder,
      client,
    ) => {
  Lwt.bind(
    client.initPromise,
    () => {
      let newRequestId = client.lastRequestId^ + 1;
      let (promise, resolver) = Lwt.task();
      Hashtbl.add(client.requestIdToReply, newRequestId, resolver);

      let finalize = () => {
        Hashtbl.remove(client.requestIdToReply, newRequestId);
        Log.tracef(m => m("Request finalized: %d", newRequestId));
      };

      let parser = json =>
        Oni_Core.Json.Decode.(
          json |> decode_value(decoder) |> Result.map_error(string_of_error)
        );

      let onError = e => {
        finalize();
        Log.warnf(m =>
          m(
            "Request %d failed with error: %s",
            newRequestId,
            Printexc.to_string(e),
          )
        );
      };

      let wrapper = json =>
        try(
          {
            finalize();
            exception ParseFailedException(string);

            switch (parser(json)) {
            | Ok(v) =>
              Log.tracef(m => m("Request %d succeeded.", newRequestId));
              Lwt.return(v);
            | Error(msg) => Lwt.fail(ParseFailedException(msg))
            };
          }
        ) {
        | e =>
          onError(e);
          Lwt.fail(e);
        };

      let () =
        notify(~usesCancellationToken, ~rpcName, ~method, ~args, client);

      Lwt.on_failure(promise, onError);
      Lwt.bind(promise, wrapper);
    },
  );
};

let terminate = ({client, _}) => Protocol.send(~message=Terminate, client);

let close = ({client, _}) => Protocol.close(client);
