/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

open EditorCoreTypes;
module Core = Oni_Core;
module Uri = Core.Uri;
open Oni_Core.Utility;
module Model = Oni_Model;

module Log = (
  val Core.Log.withNamespace("Oni2.ExtensionClientStoreConnector")
);

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;

module Workspace = Protocol.Workspace;

module ExtensionCompletionProvider = {
  let suggestionItemToCompletionItem:
    Protocol.SuggestionItem.t => Model.CompletionItem.t =
    suggestion => {
      let completionKind =
        suggestion.kind |> Option.bind(CompletionItemKind.ofInt);

      {
        label: suggestion.label,
        kind: completionKind,
        detail: suggestion.detail,
      };
    };

  let suggestionsToCompletionItems:
    option(Protocol.Suggestions.t) => list(Model.CompletionItem.t) =
    fun
    | Some(suggestions) =>
      List.map(suggestionItemToCompletionItem, suggestions)
    | None => [];

  let create =
      (
        client: ExtHostClient.t,
        {id, selector}: Protocol.SuggestProvider.t,
        (buffer, _completionMeet, location),
      ) =>
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Core.Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);
        ExtHostClient.provideCompletions(id, uri, position, client)
        |> Lwt.map(suggestionsToCompletionItems);
      },
    );
};

module ExtensionDefinitionProvider = {
  let definitionToModel = def => {
    let Protocol.DefinitionLink.{uri, range, originSelectionRange} = def;
    let Range.{start, _} = Protocol.OneBasedRange.toRange(range);

    let originRange =
      originSelectionRange |> Option.map(Protocol.OneBasedRange.toRange);

    Model.LanguageFeatures.DefinitionResult.create(
      ~originRange,
      ~uri,
      ~location=start,
    );
  };

  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Core.Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);
        ExtHostClient.provideDefinition(id, uri, position, client)
        |> Lwt.map(definitionToModel);
      },
    );
  };
};

module ExtensionDocumentHighlightProvider = {
  let definitionToModel = (highlights: list(Protocol.DocumentHighlight.t)) => {
    highlights
    |> List.map(highlights =>
         Protocol.OneBasedRange.toRange(
           Protocol.DocumentHighlight.(highlights.range),
         )
       );
  };

  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Core.Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);

        ExtHostClient.provideDocumentHighlights(id, uri, position, client)
        |> Lwt.map(definitionToModel);
      },
    );
  };
};

module ExtensionFindAllReferencesProvider = {
  let create =
      (client, {id, selector}: Protocol.BasicProvider.t, (buffer, location)) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Core.Buffer.getUri(buffer);
        let position = Protocol.OneBasedPosition.ofPosition(location);

        ExtHostClient.provideReferences(id, uri, position, client);
      },
    );
  };
};

module ExtensionDocumentSymbolProvider = {
  let create =
      (client, {id, selector, _}: Protocol.DocumentSymbolProvider.t, buffer) => {
    ProviderUtility.runIfSelectorPasses(
      ~buffer,
      ~selector,
      () => {
        let uri = Core.Buffer.getUri(buffer);
        ExtHostClient.provideDocumentSymbols(id, uri, client);
      },
    );
  };
};

let start = (extensions, setup: Core.Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let onExtHostClosed = () => Log.info("ext host closed");

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

  let onRegisterDefinitionProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let definitionProvider =
      ExtensionDefinitionProvider.create(client, provider);
    dispatch(
      Oni_Model.Actions.LanguageFeature(
        Model.LanguageFeatures.DefinitionProviderAvailable(
          id,
          definitionProvider,
        ),
      ),
    );
    Log.infof(m => m("Registered suggest provider with ID: %n", provider.id));
  };

  let onRegisterDocumentSymbolProvider = (client, provider) => {
    let id =
      Protocol.DocumentSymbolProvider.(
        "exthost." ++ string_of_int(provider.id)
      );
    let documentSymbolProvider =
      ExtensionDocumentSymbolProvider.create(client, provider);
    dispatch(
      Oni_Model.Actions.LanguageFeature(
        Model.LanguageFeatures.DocumentSymbolProviderAvailable(
          id,
          documentSymbolProvider,
        ),
      ),
    );
  };

  let onRegisterReferencesProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let findAllReferencesProvider =
      ExtensionFindAllReferencesProvider.create(client, provider);
    dispatch(
      Oni_Model.Actions.LanguageFeature(
        Model.LanguageFeatures.FindAllReferencesProviderAvailable(
          id,
          findAllReferencesProvider,
        ),
      ),
    );
  };

  let onRegisterDocumentHighlightProvider = (client, provider) => {
    let id =
      Protocol.BasicProvider.("exthost." ++ string_of_int(provider.id));
    let documentHighlightProvider =
      ExtensionDocumentHighlightProvider.create(client, provider);
    dispatch(
      Oni_Model.Actions.LanguageFeature(
        Model.LanguageFeatures.DocumentHighlightProviderAvailable(
          id,
          documentHighlightProvider,
        ),
      ),
    );
    Log.infof(m =>
      m("Registered document highlight provider with ID: %n", provider.id)
    );
  };

  let onRegisterSuggestProvider = (client, provider) => {
    let id =
      Protocol.SuggestProvider.("exthost." ++ string_of_int(provider.id));
    let completionProvider =
      ExtensionCompletionProvider.create(client, provider);
    dispatch(
      Oni_Model.Actions.LanguageFeature(
        Model.LanguageFeatures.CompletionProviderAvailable(
          id,
          completionProvider,
        ),
      ),
    );
    Log.infof(m => m("Registered suggest provider with ID: %n", provider.id));
  };

  let onOutput = Log.info;

  let onDidActivateExtension = id => {
    dispatch(Model.Actions.Extension(Model.Extensions.Activated(id)));
  };

  let onShowMessage = message => {
    dispatch(
      Oni_Model.Actions.ShowNotification(
        Oni_Model.Notification.create(~title="Extension", ~message, ()),
      ),
    );
  };

  let initData = ExtHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtHostClient.start(
      ~initialWorkspace=Workspace.fromPath(Sys.getcwd()),
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onStatusBarSetEntry,
      ~onDiagnosticsClear,
      ~onDiagnosticsChangeMany,
      ~onDidActivateExtension,
      ~onRegisterDefinitionProvider,
      ~onRegisterDocumentHighlightProvider,
      ~onRegisterDocumentSymbolProvider,
      ~onRegisterReferencesProvider,
      ~onRegisterSuggestProvider,
      ~onShowMessage,
      ~onOutput,
      setup,
    );

  let _bufferMetadataToModelAddedDelta =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    switch (bm.filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Log.info("Creating model for filetype: " ++ ft);
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

  let modelChangedEffect = (buffers: Model.Buffers.t, bu: Core.BufferUpdate.t) =>
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

          let uri = Core.Buffer.getUri(v);

          ExtHostClient.updateDocument(
            uri,
            modelChangedEvent,
            true,
            extHostClient,
          );
        })
      }
    );

  let getAndDispatchCompletions =
      (~languageFeatures, ~buffer, ~meet, ~location, ()) => {
    let completionPromise =
      Model.LanguageFeatures.requestCompletions(
        ~buffer,
        ~meet,
        ~location,
        languageFeatures,
      );

    Lwt.on_success(completionPromise, completions => {
      dispatch(Model.Actions.CompletionAddItems(meet, completions))
    });
  };

  let checkCompletionsEffect = state =>
    Isolinear.Effect.create(~name="exthost.checkCompletions", () => {
      Model.Selectors.getActiveBuffer(state)
      |> Option.iter(buf => {
           let uri = Core.Buffer.getUri(buf);
           let maybeMeet = Model.Completions.getMeet(state.completions);

           let maybeLocation =
             maybeMeet |> Option.map(Model.CompletionMeet.getLocation);

           let request = (meet: Model.CompletionMeet.t, location: Location.t) => {
             Log.infof(m =>
               m(
                 "Completions - requesting at %s for %s",
                 Core.Uri.toString(uri),
                 Location.show(location),
               )
             );
             let languageFeatures = state.languageFeatures;
             let () =
               getAndDispatchCompletions(
                 ~languageFeatures,
                 ~buffer=buf,
                 ~meet,
                 ~location,
                 (),
               );
             ();
           };

           Option.iter2(request, maybeMeet, maybeLocation);
         })
    });

  let executeContributedCommandEffect = cmd =>
    Isolinear.Effect.create(~name="exthost.executeContributedCommand", () => {
      ExtHostClient.executeContributedCommand(cmd, extHostClient)
    });

  let discoveredExtensionsEffect = extensions =>
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.discoverExtensions", dispatch =>
      dispatch(
        Model.Actions.Extension(Model.Extensions.Discovered(extensions)),
      )
    );

  let registerQuitCleanupEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.registerQuitCleanup", dispatch =>
      dispatch(
        Model.Actions.RegisterQuitCleanup(
          () => ExtHostClient.close(extHostClient),
        ),
      )
    );

  let changeWorkspaceEffect = path =>
    Isolinear.Effect.create(~name="exthost.changeWorkspace", () => {
      ExtHostClient.acceptWorkspaceData(
        Workspace.fromPath(path),
        extHostClient,
      )
    });

  let updater = (state: Model.State.t, action) =>
    switch (action) {
    | Model.Actions.Init => (
        state,
        Isolinear.Effect.batch([
          registerQuitCleanupEffect,
          discoveredExtensionsEffect(extensions),
        ]),
      )
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu),
      )
    | Model.Actions.CommandExecuteContributed(cmd) => (
        state,
        executeContributedCommandEffect(cmd),
      )
    | Model.Actions.CompletionStart(_completionMeet) => (
        state,
        checkCompletionsEffect(state),
      )
    | Model.Actions.VimDirectoryChanged(path) => (
        state,
        changeWorkspaceEffect(path),
      )
    | Model.Actions.BufferEnter(bm, fileTypeOpt) => (
        state,
        sendBufferEnterEffect(bm, fileTypeOpt),
      )
    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
