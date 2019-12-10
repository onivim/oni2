/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

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

  let onRegisterSuggestProvider = (sp: LanguageFeatures.SuggestProvider.t) => {
    Log.infof(m => m("Registered suggest provider with ID: %n", sp.id));
    dispatch(Oni_Model.Actions.LanguageFeatureRegisterSuggestProvider(sp));

    // TODO: Move
    dispatch(
      Oni_Model.Actions.LanguageFeatureRegisterDefinitionProvider(
        (_buf, _p) => {
          Log.info("Sending definiton");
          Some(
            Lwt.return(
              Extensions.LanguageFeatures.DefinitionResult.create(
                ~uri=Core.Uri.fromPath("/Users/bryphe/revery/package.json"),
                ~position=Core.Position.ofInt0(5, 5),
              ),
            ),
          );
        },
      ),
    );
  };

  let onOutput = Log.info;

  let onDidActivateExtension = id => {
    dispatch(Model.Actions.ExtensionActivated(id));
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

  let suggestionItemToCompletionItem:
    Protocol.SuggestionItem.t => Model.Actions.completionItem =
    suggestion => {
      let completionKind =
        suggestion.kind |> Option.bind(CompletionItemKind.ofInt);

      {
        completionLabel: suggestion.label,
        completionKind,
        completionDetail: suggestion.detail,
      };
    };

  let suggestionsToCompletionItems = (suggestions: Protocol.Suggestions.t) =>
    List.map(suggestionItemToCompletionItem, suggestions);

  let getAndDispatchCompletions =
      (~languageFeatures, ~fileType, ~uri, ~completionMeet, ~position, ()) => {
    let providers =
      LanguageFeatures.getSuggestProviders(fileType, languageFeatures);

    providers
    |> List.iter((provider: LanguageFeatures.SuggestProvider.t) => {
         Log.infof(m =>
           m(
             "Completions - getting completions for suggest provider: %n",
             provider.id,
           )
         );
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
               | None => Log.info("No completions for provider")
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
          open Model.Actions;

          let uri = Core.Buffer.getUri(buf);
          let position =
            Protocol.OneBasedPosition.ofInt1(
              ~lineNumber=
                completionMeet.completionMeetLine |> Core.Index.toInt1,
              ~column=completionMeet.completionMeetColumn |> Core.Index.toInt1,
              (),
            );
          Log.infof(m =>
            m(
              "Completions - requesting at %s for %s",
              Core.Uri.toString(uri),
              Protocol.OneBasedPosition.show(position),
            )
          );
          let languageFeatures = state.languageFeatures;
          getAndDispatchCompletions(
            ~languageFeatures,
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

  let executeContributedCommandEffect = cmd =>
    Isolinear.Effect.create(~name="exthost.executeContributedCommand", () => {
      ExtHostClient.executeContributedCommand(cmd, extHostClient)
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

  let changeWorkspaceEffect = path =>
    Isolinear.Effect.create(~name="exthost.changeWorkspace", () => {
      ExtHostClient.acceptWorkspaceData(
        Workspace.fromPath(path),
        extHostClient,
      )
    });

  let updater = (state: Model.State.t, action) =>
    switch (action) {
    | Model.Actions.Init => (state, registerQuitCleanupEffect)
    | Model.Actions.BufferUpdate(bu) => (
        state,
        modelChangedEffect(state.buffers, bu),
      )
    | Model.Actions.CommandExecuteContributed(cmd) => (
        state,
        executeContributedCommandEffect(cmd),
      )
    | Model.Actions.CompletionStart(completionMeet) => (
        state,
        checkCompletionsEffect(completionMeet, state),
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
