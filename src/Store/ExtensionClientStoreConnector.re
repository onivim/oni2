/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStore"));

open Oni_Extensions;
module Extensions = Oni_Extensions;
module Protocol = Extensions.ExtHostProtocol;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Workspace = Protocol.Workspace;

let start = (extensions, extHostClient) => {
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

  let modelChangedEffect = (buffers: Buffers.t, update: BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Buffers.getBuffer(update.id, buffers)) {
      | None => ()
      | Some(buffer) =>
        Oni_Core.Log.perf("exthost.bufferUpdate", () => {
          let modelContentChange =
            Protocol.ModelContentChange.ofBufferUpdate(
              update,
              Protocol.Eol.default,
            );
          let modelChangedEvent =
            Protocol.ModelChangedEvent.create(
              ~changes=[modelContentChange],
              ~eol=Protocol.Eol.default,
              ~versionId=update.version,
              (),
            );

          ExtHostClient.updateDocument(
            Buffer.getUri(buffer),
            modelChangedEvent,
            ~dirty=Buffer.isModified(buffer),
            extHostClient,
          );
        })
      }
    );

  let executeContributedCommandEffect = cmd =>
    Isolinear.Effect.create(~name="exthost.executeContributedCommand", () => {
      ExtHostClient.executeContributedCommand(cmd, extHostClient)
    });

  let gitRefreshEffect = (scm: Feature_SCM.model) =>
    if (scm == Feature_SCM.initial) {
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

               dispatch(Actions.GotOriginalContent({bufferId, lines}));
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
        dispatch(Actions.GotDecorations({handle, uri, decorations}))
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
          modelChangedEffect(state.buffers, bu.update),
        ]),
      )

    | Actions.BufferSaved(_) => (
        state,
        Isolinear.Effect.batch([gitRefreshEffect(state.scm)]),
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
            Feature_SCM.Effects.getOriginalUri(
              extHostClient, state.scm, path, uri =>
              Actions.GotOriginalUri({bufferId: metadata.id, uri})
            ),
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

    | GotOriginalUri({bufferId, uri}) => (
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

    | GotOriginalContent({bufferId, lines}) => (
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

    | NewDecorationProvider({handle, label}) => (
        {
          ...state,
          decorationProviders: [
            DecorationProvider.{handle, label},
            ...state.decorationProviders,
          ],
        },
        Isolinear.Effect.none,
      )

    | LostDecorationProvider({handle}) => (
        {
          ...state,
          decorationProviders:
            List.filter(
              (it: DecorationProvider.t) => it.handle != handle,
              state.decorationProviders,
            ),
        },
        Isolinear.Effect.none,
      )

    | DecorationsChanged({handle, uris}) => (
        state,
        Isolinear.Effect.batch(
          uris |> List.map(provideDecorationsEffect(handle)),
        ),
      )

    | GotDecorations({handle, uri, decorations}) => (
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
                        (it: Decoration.t) => it.handle != handle,
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

    | ExtMessageReceived(message) => (
        state,
        Feature_Notification.Effects.create(message)
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      )

    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
