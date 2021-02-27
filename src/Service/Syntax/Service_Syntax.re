open EditorCoreTypes;
module Core = Oni_Core;
module Syntax = Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module OptionEx = Core.Utility.OptionEx;

module Log = (val Core.Log.withNamespace("Oni2.Service_Syntax"));

module BufferTracker =
  Oni_Core.BufferTracker.Make({});

module Constants = {
  let defaultScope = "source.text";
};

[@deriving show({with_path: false})]
type serverMsg =
  | ServerStarted
  | ServerInitialized([@opaque] Oni_Syntax_Client.t)
  | ServerFailedToStart(string)
  | ServerClosed;

type bufferMsg =
  | ReceivedHighlights(list(Protocol.TokenUpdate.t));

module Internal = {
  let bufferToHighlightEvent:
    Hashtbl.t(int, Revery.Event.t(list(Protocol.TokenUpdate.t))) =
    Hashtbl.create(16);

  let subscribe = (bufferId, tokenUpdateCallback) => {
    // Create event if not already available for buffer
    let tokenUpdateEvent =
      switch (Hashtbl.find_opt(bufferToHighlightEvent, bufferId)) {
      | None =>
        let evt = Revery.Event.create();
        Hashtbl.add(bufferToHighlightEvent, bufferId, evt);
        evt;
      | Some(evt) => evt
      };

    Revery.Event.subscribe(tokenUpdateEvent, tokenUpdateCallback);
  };

  let notifyTokensReceived = (bufferId, tokenUpdate) => {
    switch (Hashtbl.find_opt(bufferToHighlightEvent, bufferId)) {
    | None => Log.warnf(m => m("No listener for bufferId: %d", bufferId))
    | Some(listener) => Revery.Event.dispatch(listener, tokenUpdate)
    };
  };
};

module Sub = {
  type serverParams = {
    id: string,
    grammarInfo: Exthost.GrammarInfo.t,
    setup: Core.Setup.t,
    tokenTheme: Syntax.TokenTheme.t,
    useTreeSitter: bool,
  };

  module SyntaxServerSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = serverMsg;

      type nonrec params = serverParams;

      type state = {
        client: result(Oni_Syntax_Client.t, string),
        lastSyncedTokenTheme: option(Syntax.TokenTheme.t),
        lastTreeSitterSetting: option(bool),
      };

      let name = "SyntaxSubscription";
      let id = params => params.id;

      let init = (~params, ~dispatch) => {
        Log.info("Init called");
        let pendingResult = ref(None);
        let clientResult =
          Oni_Syntax_Client.start(
            ~onConnected=
              () => {
                Log.info("onConnected");
                pendingResult^
                |> Option.iter(server => dispatch(ServerInitialized(server)));
              },
            ~onClose=_ => dispatch(ServerClosed),
            ~onHighlights=
              (~bufferId, ~tokens) => {
                Internal.notifyTokensReceived(bufferId, tokens)
              },
            ~onHealthCheckResult=_ => (),
            params.grammarInfo,
            params.setup,
          );

        switch (clientResult) {
        | Ok(_) => dispatch(ServerStarted)
        | Error(msg) => dispatch(ServerFailedToStart(msg))
        };

        pendingResult := Result.to_option(clientResult);

        {
          client: clientResult,
          lastSyncedTokenTheme: None,
          lastTreeSitterSetting: None,
        };
      };

      let compare: ('a, option('a)) => bool =
        (v, opt) => {
          switch (opt) {
          | None => false
          | Some(innerVal) => innerVal === v
          };
        };

      let syncTokenTheme = (tokenTheme, state) =>
        if (!compare(tokenTheme, state.lastSyncedTokenTheme)) {
          state.client
          |> Result.map(client => {
               Oni_Syntax_Client.notifyThemeChanged(client, tokenTheme);
               {...state, lastSyncedTokenTheme: Some(tokenTheme)};
             })
          |> Result.value(~default=state);
        } else {
          state;
        };

      let syncUseTreeSitter = (useTreeSitter, state) =>
        if (!compare(useTreeSitter, state.lastTreeSitterSetting)) {
          state.client
          |> Result.map(client => {
               Oni_Syntax_Client.notifyTreeSitterChanged(
                 ~useTreeSitter,
                 client,
               );
               {...state, lastTreeSitterSetting: Some(useTreeSitter)};
             })
          |> Result.value(~default=state);
        } else {
          state;
        };

      let update = (~params, ~state, ~dispatch as _) => {
        state
        |> syncTokenTheme(params.tokenTheme)
        |> syncUseTreeSitter(params.useTreeSitter);
      };

      let dispose = (~params as _, ~state) => {
        state.client |> Result.iter(Oni_Syntax_Client.close);
      };
    });

  let server = (~useTreeSitter, ~grammarInfo, ~setup, ~tokenTheme) => {
    SyntaxServerSubscription.create({
      id: "syntax-highligher",
      useTreeSitter,
      grammarInfo,
      setup,
      tokenTheme,
    });
  };

  type bufferParams = {
    client: Oni_Syntax_Client.t,
    buffer: Core.Buffer.t,
    scope: string,
    visibleRanges: list(Range.t),
  };

  module BufferSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = bufferMsg;
      type nonrec params = bufferParams;

      type state = {
        lastScope: string,
        lastVisibleRanges: list(Range.t),
        unsubscribe: unit => unit,
      };

      let name = "BufferSubscription";
      let id = params => {
        let bufferId = params.buffer |> Core.Buffer.getId |> string_of_int;
        bufferId;
      };

      let init = (~params, ~dispatch) => {
        let bufferId = Core.Buffer.getId(params.buffer);
        BufferTracker.startTracking(bufferId);
        let unsubscribe =
          Internal.subscribe(bufferId, tokenUpdates => {
            dispatch(ReceivedHighlights(tokenUpdates))
          });

        Log.infof(m => m("Starting buffer subscription for: %d", bufferId));

        Oni_Syntax_Client.startHighlightingBuffer(
          ~scope=params.scope,
          ~bufferId,
          ~visibleRanges=params.visibleRanges,
          ~lines=Core.Buffer.getLines(params.buffer),
          params.client,
        );

        {
          lastVisibleRanges: params.visibleRanges,
          unsubscribe,
          lastScope: params.scope,
        };
      };

      let update = (~params, ~state, ~dispatch as _) => {
        let currentVisibleRanges = state.lastVisibleRanges;

        if (state.lastScope != params.scope) {
          let bufferId = Core.Buffer.getId(params.buffer);
          Oni_Syntax_Client.stopHighlightingBuffer(~bufferId, params.client);
          Oni_Syntax_Client.startHighlightingBuffer(
            ~scope=params.scope,
            ~bufferId,
            ~visibleRanges=params.visibleRanges,
            ~lines=Core.Buffer.getLines(params.buffer),
            params.client,
          );
        } else if (currentVisibleRanges != params.visibleRanges) {
          Oni_Syntax_Client.notifyBufferVisibilityChanged(
            ~bufferId=Core.Buffer.getId(params.buffer),
            ~ranges=params.visibleRanges,
            params.client,
          );
        };

        {
          ...state,
          lastVisibleRanges: params.visibleRanges,
          lastScope: params.scope,
        };
      };

      let dispose = (~params, ~state) => {
        state.unsubscribe();
        let bufferId = Core.Buffer.getId(params.buffer);
        BufferTracker.stopTracking(bufferId);
        Log.infof(m => m("Stopping buffer subscription for: %d", bufferId));
        Oni_Syntax_Client.stopHighlightingBuffer(~bufferId, params.client);
      };
    });

  let buffer = (~client, ~buffer, ~languageInfo, ~visibleRanges) => {
    let scope =
      buffer
      |> Core.Buffer.getFileType
      |> Core.Buffer.FileType.toString
      |> Exthost.LanguageInfo.getScopeFromLanguage(languageInfo)
      |> Option.value(~default=Constants.defaultScope);

    BufferSubscription.create({client, buffer, scope, visibleRanges});
  };
};

module Effect = {
  let bufferUpdate = (~client, ~bufferUpdate: Core.BufferUpdate.t) => {
    Isolinear.Effect.create(~name="service.syntax.bufferUpdate", () => {
      let bufferId = bufferUpdate.id;
      if (BufferTracker.isTracking(bufferId)) {
        Oni_Syntax_Client.notifyBufferUpdate(~bufferUpdate, client);
      } else {
        ();
      };
    });
  };
};
