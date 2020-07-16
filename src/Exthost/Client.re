module Protocol = Exthost_Protocol;
module Extension = Exthost_Extension;

type t = {
  client: Protocol.t,
  lastRequestId: ref(int),
  requestIdToReply: Hashtbl.t(int, Lwt.u(Yojson.Safe.json)),
  initPromise: Lwt.t(unit),
};

exception ReplyError(string);

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
      ~handler: Msg.t => Lwt.t(Reply.t),
      ~onError: string => unit,
      (),
    ) => {
  let (initPromise, initResolver) = Lwt.task();
  let protocolClient: ref(option(Protocol.t)) = ref(None);
  let lastRequestId = ref(0);
  let requestIdToReply = Hashtbl.create(128);
  let send = message =>
    switch (protocolClient^) {
    | None =>
      Log.warnf(m =>
        m(
          "Tried to send message before protocolClient: %s, could not send",
          Protocol.Message.Outgoing.show(message),
        )
      )
    | Some(protocol) =>
      Log.infof(m =>
        m("Sending message: %s", Protocol.Message.Outgoing.show(message))
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
        ignore(handler(Ready): Lwt.t(Reply.t));

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
        ignore(handler(Initialized): Lwt.t(Reply.t));

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
      | Incoming.ReplyError({payload, requestId}) =>
        let message =
          switch (payload) {
          | Message(str) => str
          | Empty => "Unknown / Empty"
          };
        Hashtbl.find_opt(requestIdToReply, requestId)
        |> Option.iter(resolver => {
             Lwt.wakeup_exn(resolver, ReplyError(message))
           });
        Hashtbl.remove(requestIdToReply, requestId);
        onError(message);
      | Incoming.RequestJSONArgs({requestId, rpcId, method, args, _}) =>
        Log.tracef(m =>
          m("RequestJSONArgs: %d %d %s", requestId, rpcId, method)
        );
        let req = Handlers.handle(rpcId, method, args);
        switch (req) {
        | Ok(msg) =>
          let reply = handler(msg);

          let sendReply = (reply: Reply.t) => {
            switch (reply) {
            | Nothing =>
              Log.tracef(m => m("Not responding to request %d", requestId))
            | OkEmpty =>
              Log.tracef(m =>
                m("Responding to request %d with OkEmpty", requestId)
              );
              send(ReplyOKEmpty({requestId: requestId}));

            | OkJson({json}) =>
              Log.tracef(m =>
                m("Responding to request %d with OkJson", requestId)
              );
              send(ReplyOKJSON({requestId, json}));
            | OkBuffer({bytes}) =>
              Log.tracef(m =>
                m("Responding to request %d with OkBuffer", requestId)
              );
              send(ReplyOKBuffer({requestId, bytes}));
            | ErrorMessage({message}) =>
              Log.tracef(m =>
                m(
                  "Responding to request %d with error: %s",
                  requestId,
                  message,
                )
              );
              send(Outgoing.ReplyError({requestId, error: message}));
            };
          };

          let sendError = (error: exn) => {
            sendReply(Reply.error(Printexc.to_string(error)));
          };

          Lwt.on_any(reply, sendReply, sendError);
        | Error(msg) => onError(msg)
        };
      | Incoming.ReplyOk({requestId, payload}) =>
        Hashtbl.find_opt(requestIdToReply, requestId)
        |> Option.iter(resolver => {
             switch (payload) {
             | Json(json) => Lwt.wakeup(resolver, json)
             | Empty =>
               Log.tracef(m =>
                 m("Got empty payload for requestId: %d", requestId)
               );
               Lwt.wakeup(resolver, `Null);
             | Bytes(bytes) =>
               Log.warnf(m =>
                 m(
                   "Got %d bytes for requestId: %d, but bytes handler is not implemented",
                   Bytes.length(bytes),
                   requestId,
                 )
               )
             }
           });
        Hashtbl.remove(requestIdToReply, requestId);
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
  Log.tracef(m =>
    m(
      "Sending request to %s:%s with args %s",
      rpcName,
      method,
      args |> Yojson.Safe.to_string,
    )
  );
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

      let parser = json => {
        Oni_Core.Json.Decode.(
          json |> decode_value(decoder) |> Result.map_error(string_of_error)
        );
      };

      let onError = e => {
        finalize();
        Log.errorf(m =>
          m(
            "Request %d for %s failed with error: %s",
            newRequestId,
            method,
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
        | e => Lwt.fail(e)
        };

      let () =
        notify(~usesCancellationToken, ~rpcName, ~method, ~args, client);

      let out = Lwt.bind(promise, wrapper);
      Lwt.on_failure(out, onError);
      out;
    },
  );
};

let terminate = ({client, _}) => Protocol.send(~message=Terminate, client);

let close = ({client, _}) => Protocol.close(client);
