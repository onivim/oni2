/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

module Core = Oni_Core;
open Oni_Core.Utility;
module Model = Oni_Model;

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;

let start = (extensions, setup: Core.Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let onExtHostClosed = () => Core.Log.info("ext host closed");

  // Keep track of the available language features
  // TODO: Keep track of this in the application state / store,
  // if needed more globally?

  let languageFeatures = ref(LanguageFeatures.create());

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtHostInitData.ExtensionInfo.ofScannedExtension(ext)
       );

  let onDiagnosticsClear = owner => {
    dispatch(Model.Actions.DiagnosticsClear(owner));
  };

  let onDiagnosticsChangeMany =
      (diagCollection: Protocol.DiagnosticsCollection.t) => {
    let protocolDiagToDiag: Protocol.Diagnostic.t => Model.Diagnostic.t =
      d => {
        let range = Protocol.OneBasedRange.toRange(d.range);
        let message = d.message;
        Model.Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Protocol.Diagnostics.t) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Model.Actions.DiagnosticsSet(uri, diagCollection.name, diagnostics);
    };

    diagCollection.perFileDiagnostics
    |> List.map(f)
    |> List.iter(a => dispatch(a));
  };

  let onStatusBarSetEntry = ((id, text, alignment, priority)) => {
    dispatch(
      Model.Actions.StatusBarAddItem(
        Model.StatusBarModel.Item.create(
          ~id,
          ~text,
          ~alignment=Model.StatusBarModel.Alignment.ofInt(alignment),
          ~priority,
          (),
        ),
      ),
    );
  };

  let onRegisterSuggestProvider = (sp: LanguageFeatures.SuggestProvider.t) => {
    Core.Log.info(
      "Registered suggest provider with ID: " ++ string_of_int(sp.id),
    );
    languageFeatures :=
      LanguageFeatures.registerSuggestProvider(sp, languageFeatures^);
  };

  let onOutput = msg => {
    Core.Log.info("[ExtHost]: " ++ msg);
  };

  let onDidActivateExtension = id => {
    dispatch(Model.Actions.ExtensionActivated(id));
  };

  let initData = ExtHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtHostClient.start(
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onStatusBarSetEntry,
      ~onDiagnosticsClear,
      ~onDiagnosticsChangeMany,
      ~onDidActivateExtension,
      ~onRegisterSuggestProvider,
      ~onOutput,
      setup,
    );

  let _bufferMetadataToModelAddedDelta =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    switch (bm.filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Core.Log.info("Creating model for filetype: " ++ ft);
      Some(
        Protocol.ModelAddedDelta.create(
          ~uri=Core.Uri.fromPath(fp),
          ~versionId=bm.version,
          ~lines=[""],
          ~modeId=ft,
          ~isDirty=true,
          (),
        ),
      );
    | _ => None
    };

  let pumpEffect =
    Isolinear.Effect.create(~name="exthost.pump", () =>
      ExtHostClient.pump(extHostClient)
    );

  let activatedFileTypes: Hashtbl.t(string, bool) = Hashtbl.create(16);

  let activateFileType = (fileType: option(string)) =>
    fileType
    |> Option.iter(ft =>
         Hashtbl.find_opt(activatedFileTypes, ft)
         // If no entry, we haven't activated yet
         |> Option.iter_none(() => {
              ExtHostClient.activateByEvent(
                "onLanguage:" ++ ft,
                extHostClient,
              );
              Hashtbl.add(activatedFileTypes, ft, true);
            })
       );

  let sendBufferEnterEffect =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    Isolinear.Effect.create(~name="exthost.bufferEnter", () =>
      switch (_bufferMetadataToModelAddedDelta(bm, fileType)) {
      | None => ()
      | Some((v: Protocol.ModelAddedDelta.t)) =>
        activateFileType(fileType);
        ExtHostClient.addDocument(v, extHostClient);
      }
    );

  let modelChangedEffect =
      (buffers: Model.Buffers.t, bu: Core.Types.BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Model.Buffers.getBuffer(bu.id, buffers)) {
      | None => ()
      | Some(v) =>
        Core.Log.perf("exthost.bufferUpdate", () => {
          let modelContentChange =
            Protocol.ModelContentChange.ofBufferUpdate(
              bu,
              Protocol.Eol.default,
            );
          let modelChangedEvent =
            Protocol.ModelChangedEvent.create(
              ~changes=[modelContentChange],
              ~eol=Protocol.Eol.default,
              ~versionId=bu.version,
              (),
            );

          let uri = Model.Buffer.getUri(v);

          ExtHostClient.updateDocument(
            uri,
            modelChangedEvent,
            true,
            extHostClient,
          );
        })
      }
    );

  let suggestionItemToCompletionItem:
    Protocol.SuggestionItem.t => Model.Actions.completionItem =
    suggestion => {
      {
        completionLabel: suggestion.label,
        completionKind: CompletionKind.Text,
        completionDetail: None,
      };
    };

  let suggestionsToCompletionItems = (suggestions: Protocol.Suggestions.t) =>
    List.map(suggestionItemToCompletionItem, suggestions);

  let getAndDispatchCompletions =
      (~fileType, ~uri, ~completionMeet, ~position, ()) => {
    let providers =
      LanguageFeatures.getSuggestProviders(fileType, languageFeatures^);

    providers
    |> List.iter((provider: LanguageFeatures.SuggestProvider.t) => {
         let completionPromise: Lwt.t(option(Protocol.Suggestions.t)) =
           ExtHostClient.getCompletions(
             provider.id,
             uri,
             position,
             extHostClient,
           );

         let _ =
           Lwt.bind(
             completionPromise,
             completions => {
               switch (completions) {
               | None => Core.Log.info("No completions for provider")
               | Some(completions) =>
                 let completionItems =
                   suggestionsToCompletionItems(completions);
                 dispatch(
                   Model.Actions.CompletionAddItems(
                     completionMeet,
                     completionItems,
                   ),
                 );
               };
               Lwt.return();
             },
           );
         ();
       });
  };

  let checkCompletionsEffect = (completionMeet, state) =>
    Isolinear.Effect.create(~name="exthost.checkCompletions", () => {
      Model.Selectors.withActiveBufferAndFileType(
        state,
        (buf, fileType) => {
          let uri = Model.Buffer.getUri(buf);
          let position =
            Protocol.OneBasedPosition.ofInt1(~lineNumber=1, ~column=2, ());
          getAndDispatchCompletions(
            ~fileType,
            ~uri,
            ~completionMeet,
            ~position,
            (),
          );
          ();
        },
      )
    });

  let registerQuitCleanupEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.registerQuitCleanup", dispatch =>
      dispatch(
        Model.Actions.RegisterQuitCleanup(
          () => ExtHostClient.close(extHostClient),
        ),
      )
    );

  let updater = (state: Model.State.t, action) =>
    switch (action) {
    | Model.Actions.Init => (state, registerQuitCleanupEffect)
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu),
      )
    | Model.Actions.CompletionStart(completionMeet) => (
        state,
        checkCompletionsEffect(completionMeet, state),
      )
    | Model.Actions.BufferEnter(bm, fileTypeOpt) => (
        state,
        sendBufferEnterEffect(bm, fileTypeOpt),
      )
    | Model.Actions.Tick(_) => (state, pumpEffect)
    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
