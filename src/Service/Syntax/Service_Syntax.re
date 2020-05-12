open EditorCoreTypes;
module Core = Oni_Core;
open Utility;
module Syntax = Oni_Syntax;
module Protocol = Oni_Syntax.Protocol;
module Ext = Oni_Extensions;
module OptionEx = Core.Utility.OptionEx;

module Log = (val Core.Log.withNamespace("Oni2.Service_Syntax"));

[@deriving show({with_path: false})]
type serverMsg =
  | ServerStarted([@opaque] Oni_Syntax_Client.t)
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
    languageInfo: Ext.LanguageInfo.t,
    setup: Core.Setup.t,
    tokenTheme: Syntax.TokenTheme.t,
    configuration: Core.Configuration.t,
  };

  module SyntaxServerSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = serverMsg;

      type nonrec params = serverParams;

      type state = {
        client: result(Oni_Syntax_Client.t, string),
        lastSyncedTokenTheme: option(Syntax.TokenTheme.t),
        lastConfiguration: option(Core.Configuration.t),
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
                |> Option.iter(server => dispatch(ServerStarted(server)));
              },
            ~onClose=_ => dispatch(ServerClosed),
            ~onHighlights=
              (~bufferId, ~tokens) => {
                Internal.notifyTokensReceived(bufferId, tokens)
              },
            ~onHealthCheckResult=_ => (),
            params.languageInfo,
            params.setup,
          )
          |> Utility.ResultEx.tap(server => dispatch(ServerStarted(server)))
          |> Utility.ResultEx.tapError(msg =>
               dispatch(ServerFailedToStart(msg))
             );

        {
          client: clientResult,
          lastSyncedTokenTheme: None,
          lastConfiguration: None,
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

      let syncConfiguration = (configuration, state) =>
        if (!compare(configuration, state.lastConfiguration)) {
          state.client
          |> Result.map(client => {
               Oni_Syntax_Client.notifyConfigurationChanged(
                 client,
                 configuration,
               );
               {...state, lastConfiguration: Some(configuration)};
             })
          |> Result.value(~default=state);
        } else {
          state;
        };

      let update = (~params, ~state, ~dispatch as _) => {
        state
        |> syncTokenTheme(params.tokenTheme)
        |> syncConfiguration(params.configuration);
      };

      let dispose = (~params as _, ~state) => {
        state.client |> Result.iter(Oni_Syntax_Client.close);
      };
    });

  let server = (~configuration, ~languageInfo, ~setup, ~tokenTheme) => {
    SyntaxServerSubscription.create({
      id: "syntax-highligher",
      configuration,
      languageInfo,
      setup,
      tokenTheme,
    });
  };

  type bufferParams = {
    client: Oni_Syntax_Client.t,
    buffer: Core.Buffer.t,
    visibleRanges: list(Range.t),
  };

  module BufferSubscription =
    Isolinear.Sub.Make({
      type nonrec msg = bufferMsg;
      type nonrec params = bufferParams;

      type state = {
        lastVisibleRanges: list(Range.t),
        unsubscribe: unit => unit,
      };

      let name = "BufferSubscription";
      let id = params => {
        let bufferId = params.buffer |> Core.Buffer.getId |> string_of_int;

        let fileType =
          params.buffer
          |> Core.Buffer.getFileType
          |> Option.value(~default="(none)");

        bufferId ++ fileType;
      };

      let init = (~params, ~dispatch) => {
        let bufferId = Core.Buffer.getId(params.buffer);
        let unsubscribe =
          Internal.subscribe(bufferId, tokenUpdates => {
            dispatch(ReceivedHighlights(tokenUpdates))
          });

        Log.infof(m => m("Starting buffer subscription for: %d", bufferId));

        params.buffer
        |> Core.Buffer.getFileType
        |> Option.iter(filetype => {
             Oni_Syntax_Client.startHighlightingBuffer(
               ~filetype,
               ~bufferId,
               ~visibleRanges=params.visibleRanges,
               ~lines=Core.Buffer.getLines(params.buffer),
               params.client,
             )
           });

        {lastVisibleRanges: params.visibleRanges, unsubscribe};
      };

      let update = (~params, ~state, ~dispatch as _) => {
        let currentVisibleRanges = state.lastVisibleRanges;

        if (!
              RangeEx.areRangesEqual(
                currentVisibleRanges,
                params.visibleRanges,
              )) {
          Oni_Syntax_Client.notifyBufferVisibilityChanged(
            ~bufferId=Core.Buffer.getId(params.buffer),
            ~ranges=params.visibleRanges,
            params.client,
          );
        };

        {...state, lastVisibleRanges: params.visibleRanges};
      };

      let dispose = (~params, ~state) => {
        state.unsubscribe();
        let bufferId = Core.Buffer.getId(params.buffer);
        Log.infof(m => m("Stopping buffer subscription for: %d", bufferId));
        Oni_Syntax_Client.stopHighlightingBuffer(~bufferId, params.client);
      };
    });

  let buffer = (~client, ~buffer, ~visibleRanges) => {
    BufferSubscription.create({client, buffer, visibleRanges});
  };
};
