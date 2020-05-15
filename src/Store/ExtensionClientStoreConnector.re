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

module Extensions = Oni_Extensions;
module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

module Internal = {
  let bufferMetadataToModelAddedDelta =
      (~version, ~filePath, ~fileType: option(string)) =>
    switch (filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Log.trace("Creating model for filetype: " ++ ft);

      Some(
        Exthost.ModelAddedDelta.create(
          ~versionId=version,
          ~lines=[""],
          ~modeId=ft,
          ~isDirty=true,
          Uri.fromPath(fp),
        ),
      );
    | _ => None
    };
};

let start = (extensions, extHostClient: Exthost.Client.t) => {
  let activatedFileTypes: Hashtbl.t(string, bool) = Hashtbl.create(16);

  let activateFileType = (fileType: option(string)) =>
    fileType
    |> Option.iter(ft =>
         if (!Hashtbl.mem(activatedFileTypes, ft)) {
           // If no entry, we haven't activated yet
           Exthost.Request.ExtensionService.activateByEvent(
             ~event="onLanguage:" ++ ft,
             extHostClient,
           );
           Hashtbl.add(activatedFileTypes, ft, true);
         }
       );

  let sendBufferEnterEffect = (~version, ~filePath, ~fileType) =>
    Isolinear.Effect.create(~name="exthost.bufferEnter", () =>
      switch (
        Internal.bufferMetadataToModelAddedDelta(
          ~version,
          ~filePath,
          ~fileType,
        )
      ) {
      | None => ()
      | Some((v: Exthost.ModelAddedDelta.t)) =>
        activateFileType(fileType);
        let addedDelta =
          Exthost.DocumentsAndEditorsDelta.create(
            ~removedDocuments=[],
            ~addedDocuments=[v],
          );
        Exthost.Request.DocumentsAndEditors.acceptDocumentsAndEditorsDelta(
          ~delta=addedDelta,
          extHostClient,
        );
      }
    );

  let modelChangedEffect = (buffers: Buffers.t, update: BufferUpdate.t) =>
    Isolinear.Effect.create(~name="exthost.bufferUpdate", () =>
      switch (Buffers.getBuffer(update.id, buffers)) {
      | None => ()
      | Some(buffer) =>
        Oni_Core.Log.perf("exthost.bufferUpdate", () => {
          let modelContentChange =
            Exthost.ModelContentChange.ofBufferUpdate(
              update,
              Exthost.Eol.default,
            );
          let modelChangedEvent =
            Exthost.ModelChangedEvent.{
              changes: [modelContentChange],
              eol: Exthost.Eol.default,
              versionId: update.version,
            };

          Exthost.Request.Documents.acceptModelChanged(
            ~uri=Buffer.getUri(buffer),
            ~modelChangedEvent,
            ~isDirty=Buffer.isModified(buffer),
            extHostClient,
          );
        })
      }
    );

  let executeContributedCommandEffect = (command, arguments) =>
    Isolinear.Effect.create(~name="exthost.executeContributedCommand", () => {
      Exthost.Request.Commands.executeContributedCommand(
        ~command,
        ~arguments,
        extHostClient,
      )
    });

  let gitRefreshEffect = (scm: Feature_SCM.model) =>
    if (scm == Feature_SCM.initial) {
      Isolinear.Effect.none;
    } else {
      executeContributedCommandEffect("git.refresh", []);
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
          () => Exthost.Client.terminate(extHostClient),
        ),
      )
    );

  let changeWorkspaceEffect = path =>
    Isolinear.Effect.create(~name="exthost.changeWorkspace", () => {
      Exthost.Request.Workspace.acceptWorkspaceData(
        ~workspace=Some(Exthost.WorkspaceData.fromPath(path)),
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
             Exthost.Request.DocumentContentProvider.provideTextDocumentContent(
               ~handle,
               ~uri,
               extHostClient,
             );

           Lwt.on_success(promise, maybeContent => {
             switch (maybeContent) {
             | None => ()
             | Some(content) =>
               let lines =
                 content |> Str.(split(regexp("\r?\n"))) |> Array.of_list;

               dispatch(Actions.GotOriginalContent({bufferId, lines}));
             }
           });
         });
    });

  let provideDecorationsEffect = {
    open Exthost.Request.Decorations;
    let nextRequestId = ref(0);

    (handle, uri) =>
      Isolinear.Effect.createWithDispatch(
        ~name="exthost.provideDecorations", dispatch => {
        let requests = [{id: nextRequestId^, handle, uri}];
        incr(nextRequestId);

        let promise =
          Exthost.Request.Decorations.provideDecorations(
            ~requests,
            extHostClient,
          );

        let toCoreDecoration:
          Exthost.Request.Decorations.decoration => Oni_Core.Decoration.t =
          decoration => {
            handle,
            tooltip: decoration.title,
            letter: decoration.letter,
            color: decoration.color.id,
          };

        Lwt.on_success(
          promise,
          decorations => {
            let decorations =
              decorations
              |> IntMap.bindings
              |> List.to_seq
              |> Seq.map(snd)
              |> Seq.map(toCoreDecoration)
              |> List.of_seq;

            dispatch(Actions.GotDecorations({handle, uri, decorations}));
          },
        );
      });
  };

  let updater = (state: State.t, action: Actions.t) =>
    switch (action) {
    | Init => (
        state,
        Isolinear.Effect.batch([
          registerQuitCleanupEffect,
          discoveredExtensionsEffect(extensions),
        ]),
      )

    | BufferUpdate(bu) => (
        state,
        Isolinear.Effect.batch([
          modelChangedEffect(state.buffers, bu.update),
        ]),
      )

    | BufferSaved(_) => (
        state,
        Isolinear.Effect.batch([gitRefreshEffect(state.scm)]),
      )

    | Extension(ExecuteCommand({command, arguments})) => (
        state,
        executeContributedCommandEffect(command, arguments),
      )

    | VimDirectoryChanged(path) => (state, changeWorkspaceEffect(path))

    | BufferEnter({id, version, filePath, fileType, _}) =>
      let eff =
        switch (filePath) {
        | Some(path) =>
          Isolinear.Effect.batch([
            Feature_SCM.Effects.getOriginalUri(
              extHostClient, state.scm, path, uri =>
              Actions.GotOriginalUri({bufferId: id, uri})
            ),
            sendBufferEnterEffect(~version, ~filePath, ~fileType),
          ])

        | None => sendBufferEnterEffect(~version, ~filePath, ~fileType)
        };
      (state, eff);

    | NewTextContentProvider({handle, scheme}) => (
        {
          ...state,
          textContentProviders: [
            (handle, scheme),
            ...state.textContentProviders,
          ],
        },
        Isolinear.Effect.none,
      )

    | LostTextContentProvider({handle}) => (
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

    | ExtMessageReceived({severity, message, extensionId}) =>
      let kind: Feature_Notification.kind =
        switch (severity) {
        | Exthost.Msg.MessageService.Ignore => Info
        | Exthost.Msg.MessageService.Info => Info
        | Exthost.Msg.MessageService.Warning => Warning
        | Exthost.Msg.MessageService.Error => Error
        };

      (
        state,
        Feature_Notification.Effects.create(
          ~kind,
          ~source=?extensionId,
          message,
        )
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      );

    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
