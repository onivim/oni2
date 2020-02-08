/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Model;
open Utility;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Workspace = Protocol.Workspace;

module ExtensionCompletionProvider = {
  let suggestionItemToCompletionItem:
    Protocol.SuggestionItem.t => CompletionItem.t =
    suggestion => {
      let completionKind =
        Option.bind(suggestion.kind, CompletionItemKind.ofInt);

      {
        label: suggestion.label,
        kind: completionKind,
        detail: suggestion.detail,
      };
    };

  let suggestionsToCompletionItems:
    option(Protocol.Suggestions.t) => list(CompletionItem.t) =
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
        let uri = Buffer.getUri(buffer);
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

    LanguageFeatures.DefinitionResult.create(
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
        let uri = Buffer.getUri(buffer);
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
        let uri = Buffer.getUri(buffer);
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
        let uri = Buffer.getUri(buffer);
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
        let uri = Buffer.getUri(buffer);
        ExtHostClient.provideDocumentSymbols(id, uri, client);
      },
    );
  };
};

let start = (extensions, setup: Setup.t) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let manifests =
    List.map((ext: ExtensionScanner.t) => ext.manifest, extensions);

  let defaults = Configuration.Model.ofExtensions(manifests);
  let keys = ["reason_language_server.location"];

  let contents =
    `Assoc([
      (
        "reason_language_server",
        `Assoc([("location", `String(setup.rlsPath))]),
      ),
    ]);
  let user = Configuration.Model.create(~keys, contents);

  let initialConfiguration = Configuration.create(~defaults, ~user, ());

  let onExtHostClosed = () => Log.debug("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtHostInitData.ExtensionInfo.ofScannedExtension(ext)
       );

  let onDiagnosticsClear = owner => {
    dispatch(Actions.DiagnosticsClear(owner));
  };

  let onDiagnosticsChangeMany =
      (diagCollection: Protocol.DiagnosticsCollection.t) => {
    let protocolDiagToDiag: Protocol.Diagnostic.t => Diagnostic.t =
      d => {
        let range = Protocol.OneBasedRange.toRange(d.range);
        let message = d.message;
        Diagnostic.create(~range, ~message, ());
      };

    let f = (d: Protocol.Diagnostics.t) => {
      let diagnostics = List.map(protocolDiagToDiag, snd(d));
      let uri = fst(d);
      Actions.DiagnosticsSet(uri, diagCollection.name, diagnostics);
    };

    diagCollection.perFileDiagnostics
    |> List.map(f)
    |> List.iter(a => dispatch(a));
  };

  let onStatusBarSetEntry = ((id, text, alignment, priority)) => {
    dispatch(
      Actions.StatusBarAddItem(
        StatusBarModel.Item.create(
          ~id,
          ~text,
          ~alignment=StatusBarModel.Alignment.ofInt(alignment),
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
      Actions.LanguageFeature(
        LanguageFeatures.DefinitionProviderAvailable(id, definitionProvider),
      ),
    );
  };

  let onRegisterDocumentSymbolProvider = (client, provider) => {
    let id =
      Protocol.DocumentSymbolProvider.(
        "exthost." ++ string_of_int(provider.id)
      );
    let documentSymbolProvider =
      ExtensionDocumentSymbolProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.DocumentSymbolProviderAvailable(
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
      Actions.LanguageFeature(
        LanguageFeatures.FindAllReferencesProviderAvailable(
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
      Actions.LanguageFeature(
        LanguageFeatures.DocumentHighlightProviderAvailable(
          id,
          documentHighlightProvider,
        ),
      ),
    );
  };

  let onRegisterSuggestProvider = (client, provider) => {
    let id =
      Protocol.SuggestProvider.("exthost." ++ string_of_int(provider.id));
    let completionProvider =
      ExtensionCompletionProvider.create(client, provider);

    dispatch(
      Actions.LanguageFeature(
        LanguageFeatures.CompletionProviderAvailable(id, completionProvider),
      ),
    );
  };

  let onClientMessage = (msg: ExtHostClient.msg) =>
    switch (msg) {
    | RegisterSourceControl({handle, id, label, rootUri}) =>
      dispatch(Actions.SCM(SCM.NewProvider({handle, id, label, rootUri})))

    | UnregisterSourceControl({handle}) =>
      dispatch(Actions.SCM(SCM.LostProvider({handle: handle})))

    | RegisterSCMResourceGroup({provider, handle, id, label}) =>
      dispatch(
        Actions.SCM(SCM.NewResourceGroup({provider, handle, id, label})),
      )

    | UnregisterSCMResourceGroup({provider, handle}) =>
      dispatch(Actions.SCM(SCM.LostResourceGroup({provider, handle})))

    | SpliceSCMResourceStates({
        provider,
        group,
        start,
        deleteCount,
        additions,
      }) =>
      dispatch(
        Actions.SCM(
          SCM.ResourceStatesChanged({
            provider,
            group,
            spliceStart: start,
            deleteCount,
            additions,
          }),
        ),
      )

    | UpdateSourceControl({
        handle,
        hasQuickDiffProvider,
        count,
        commitTemplate,
      }) =>
      Option.iter(
        available =>
          dispatch(
            Actions.SCM(SCM.QuickDiffProviderChanged({handle, available})),
          ),
        hasQuickDiffProvider,
      );
      Option.iter(
        count => dispatch(Actions.SCM(SCM.CountChanged({handle, count}))),
        count,
      );
      Option.iter(
        template =>
          dispatch(
            Actions.SCM(SCM.CommitTemplateChanged({handle, template})),
          ),
        commitTemplate,
      );

    | RegisterTextContentProvider({handle, scheme}) =>
      dispatch(NewTextContentProvider({handle, scheme}))

    | UnregisterTextContentProvider({handle}) =>
      dispatch(LostTextContentProvider({handle: handle}))

    | RegisterDecorationProvider({handle, label}) =>
      dispatch(Actions.SCM(SCM.NewDecorationProvider({handle, label})))

    | UnregisterDecorationProvider({handle}) =>
      dispatch(Actions.SCM(SCM.LostDecorationProvider({handle: handle})))

    | DecorationsDidChange({handle, uris}) =>
      dispatch(Actions.SCM(SCM.DecorationsChanged({handle, uris})))
    };

  let onOutput = Log.info;

  let onDidActivateExtension = id => {
    dispatch(Actions.Extension(Oni_Model.Extensions.Activated(id)));
  };

  let onShowMessage = message => {
    dispatch(Actions.ShowNotification(Notification.create(message)));
  };

  let initData = ExtHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtHostClient.start(
      ~initialConfiguration,
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
      ~dispatch=onClientMessage,
      setup,
    );

  let _bufferMetadataToModelAddedDelta =
      (bm: Vim.BufferMetadata.t, fileType: option(string)) =>
    switch (bm.filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Log.trace("Creating model for filetype: " ++ ft);

      Some(
        Protocol.ModelAddedDelta.create(
          ~uri=Uri.fromPath(fp),
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
         if (!Hashtbl.mem(activatedFileTypes, ft)) {
           // If no entry, we haven't activated yet
           ExtHostClient.activateByEvent("onLanguage:" ++ ft, extHostClient);
           Hashtbl.add(activatedFileTypes, ft, true);
         }
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

  let modelChangedEffect = (buffers: Buffers.t, bu: BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Buffers.getBuffer(bu.id, buffers)) {
      | None => ()
      | Some(v) =>
        Oni_Core.Log.perf("exthost.bufferUpdate", () => {
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

          let uri = Buffer.getUri(v);

          ExtHostClient.updateDocument(
            uri,
            modelChangedEvent,
            true,
            extHostClient,
          );
        })
      }
    );

  let executeContributedCommandEffect = cmd =>
    Isolinear.Effect.create(~name="exthost.executeContributedCommand", () => {
      ExtHostClient.executeContributedCommand(cmd, extHostClient)
    });

  let gitRefreshEffect = (scm: SCM.t) =>
    if (scm.providers == []) {
      Isolinear.Effect.none;
    } else {
      executeContributedCommandEffect("git.refresh");
    };

  let discoveredExtensionsEffect = extensions =>
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.discoverExtensions", dispatch =>
      dispatch(
        Actions.Extension(Oni_Model.Extensions.Discovered(extensions)),
      )
    );

  let registerQuitCleanupEffect =
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.registerQuitCleanup", dispatch =>
      dispatch(
        Actions.RegisterQuitCleanup(
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

  let getOriginalUri = (bufferId, path, providers: list(SCM.Provider.t)) =>
    Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
      // Try our luck with every provider. If several returns Last-Writer-Wins
      // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
      providers
      |> List.iter((provider: SCM.Provider.t) => {
           let promise =
             ExtHostClient.provideOriginalResource(
               provider.handle,
               Uri.fromPath(path),
               extHostClient,
             );

           Lwt.on_success(promise, uri =>
             dispatch(Actions.SCM(SCM.GotOriginalUri({bufferId, uri})))
           );
         })
    });

  let getOriginalContent = (bufferId, uri, providers) =>
    Isolinear.Effect.createWithDispatch(
      ~name="scm.getOriginalSourceLines", dispatch => {
      let scheme = uri |> Uri.getScheme |> Uri.Scheme.toString;
      providers
      |> List.find_opt(((_, providerScheme)) => providerScheme == scheme)
      |> Option.iter(provider => {
           let (handle, _) = provider;
           let promise =
             ExtHostClient.provideTextDocumentContent(
               handle,
               uri,
               extHostClient,
             );

           Lwt.on_success(
             promise,
             content => {
               let lines =
                 content |> Str.(split(regexp("\r?\n"))) |> Array.of_list;

               dispatch(
                 Actions.SCM(SCM.GotOriginalContent({bufferId, lines})),
               );
             },
           );
         });
    });

  let provideDecorationsEffect = (handle, uri) =>
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.provideDecorations", dispatch => {
      let promise =
        Oni_Extensions.ExtHostClient.provideDecorations(
          handle,
          uri,
          extHostClient,
        );

      Lwt.on_success(promise, decorations =>
        dispatch(
          Actions.SCM(SCM.GotDecorations({handle, uri, decorations})),
        )
      );
    });

  let updater = (state: State.t, action) =>
    switch (action) {
    | Actions.Init => (
        state,
        Isolinear.Effect.batch([
          registerQuitCleanupEffect,
          discoveredExtensionsEffect(extensions),
        ]),
      )

    | Actions.BufferUpdate(bu) => (
        state,
        Isolinear.Effect.batch([
          modelChangedEffect(state.buffers, bu),
          gitRefreshEffect(state.scm),
        ]),
      )

    | Actions.CommandExecuteContributed(cmd) => (
        state,
        executeContributedCommandEffect(cmd),
      )

    | Actions.VimDirectoryChanged(path) => (
        state,
        changeWorkspaceEffect(path),
      )

    | Actions.BufferEnter(metadata, fileTypeOpt) =>
      let eff =
        switch (metadata.filePath) {
        | Some(path) =>
          Isolinear.Effect.batch([
            sendBufferEnterEffect(metadata, fileTypeOpt),
            getOriginalUri(metadata.id, path, state.scm.providers),
          ])

        | None => sendBufferEnterEffect(metadata, fileTypeOpt)
        };
      (state, eff);

    | Actions.NewTextContentProvider({handle, scheme}) => (
        {
          ...state,
          textContentProviders: [
            (handle, scheme),
            ...state.textContentProviders,
          ],
        },
        Isolinear.Effect.none,
      )

    | Actions.LostTextContentProvider({handle}) => (
        {
          ...state,
          textContentProviders:
            List.filter(
              ((h, _)) => h != handle,
              state.textContentProviders,
            ),
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.NewProvider({handle, id, label, rootUri})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers: [
              SCM.Provider.{
                handle,
                id,
                label,
                rootUri,
                resourceGroups: [],
                hasQuickDiffProvider: false,
                count: 0,
                commitTemplate: "",
              },
              ...state.scm.providers,
            ],
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.LostProvider({handle})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.filter(
                (it: SCM.Provider.t) => it.handle != handle,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.QuickDiffProviderChanged({handle, available})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (it: SCM.Provider.t) =>
                  it.handle == handle
                    ? {...it, hasQuickDiffProvider: available} : it,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.CountChanged({handle, count})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (it: SCM.Provider.t) =>
                  it.handle == handle ? {...it, count} : it,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.CommitTemplateChanged({handle, template})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (it: SCM.Provider.t) =>
                  it.handle == handle
                    ? {...it, commitTemplate: template} : it,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.NewResourceGroup({provider, handle, id, label})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (p: SCM.Provider.t) =>
                  p.handle == provider
                    ? {
                      ...p,
                      resourceGroups: [
                        SCM.ResourceGroup.{
                          handle,
                          id,
                          label,
                          hideWhenEmpty: false,
                          resources: [],
                        },
                        ...p.resourceGroups,
                      ],
                    }
                    : p,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.LostResourceGroup({provider, handle})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (p: SCM.Provider.t) =>
                  p.handle == provider
                    ? {
                      ...p,
                      resourceGroups:
                        List.filter(
                          (g: SCM.ResourceGroup.t) => g.handle != handle,
                          p.resourceGroups,
                        ),
                    }
                    : p,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(
        SCM.ResourceStatesChanged({
          provider,
          group,
          spliceStart,
          deleteCount,
          additions,
        }),
      ) => (
        {
          ...state,
          scm: {
            ...state.scm,
            providers:
              List.map(
                (p: SCM.Provider.t) =>
                  p.handle == provider
                    ? {
                      ...p,
                      resourceGroups:
                        List.map(
                          (g: SCM.ResourceGroup.t) =>
                            g.handle == group
                              ? {
                                ...g,
                                resources:
                                  ListEx.splice(
                                    ~start=spliceStart,
                                    ~deleteCount,
                                    ~additions,
                                    g.resources,
                                  ),
                              }
                              : g,
                          p.resourceGroups,
                        ),
                    }
                    : p,
                state.scm.providers,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.GotOriginalUri({bufferId, uri})) => (
        {
          ...state,
          buffers:
            IntMap.update(
              bufferId,
              Option.map(Buffer.setOriginalUri(uri)),
              state.buffers,
            ),
        },
        getOriginalContent(bufferId, uri, state.textContentProviders),
      )

    | Actions.SCM(SCM.GotOriginalContent({bufferId, lines})) => (
        {
          ...state,
          buffers:
            IntMap.update(
              bufferId,
              Option.map(Buffer.setOriginalLines(lines)),
              state.buffers,
            ),
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.NewDecorationProvider({handle, label})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            decorationProviders: [
              SCM.DecorationProvider.{handle, label},
              ...state.scm.decorationProviders,
            ],
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.LostDecorationProvider({handle})) => (
        {
          ...state,
          scm: {
            ...state.scm,
            decorationProviders:
              List.filter(
                (it: SCM.DecorationProvider.t) => it.handle != handle,
                state.scm.decorationProviders,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | Actions.SCM(SCM.DecorationsChanged({handle, uris})) => (
        state,
        Isolinear.Effect.batch(
          uris |> List.map(provideDecorationsEffect(handle)),
        ),
      )

    | Actions.SCM(SCM.GotDecorations({handle, uri, decorations})) => (
        {
          ...state,
          fileExplorer: {
            ...state.fileExplorer,
            decorations:
              StringMap.update(
                Uri.toFileSystemPath(uri),
                fun
                | Some(existing) => {
                    let existing =
                      List.filter(
                        (it: SCMDecoration.t) => it.handle != handle,
                        existing,
                      );
                    Some(decorations @ existing);
                  }
                | None => Some(decorations),
                state.fileExplorer.decorations,
              ),
          },
        },
        Isolinear.Effect.none,
      )

    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
