/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

open Oni_Core;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Extension.ClientStoreConnector"));

module CompletionItem = Feature_LanguageSupport.CompletionItem;
module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

let start = (extensions, extHostClient: Exthost.Client.t) => {
  let gitRefreshEffect = (scm: Feature_SCM.model) =>
    if (scm == Feature_SCM.initial) {
      Isolinear.Effect.none;
    } else {
      Service_Exthost.Effects.Commands.executeContributedCommand(
        ~command="git.refresh",
        ~arguments=[],
        extHostClient,
      );
    };

  let discoveredExtensionsEffect = extensions =>
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.discoverExtensions", dispatch =>
      dispatch(
        Actions.Extensions(Feature_Extensions.Discovered(extensions)),
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

    | BufferUpdate({update, newBuffer, triggerKey, _}) => (
        state,
        Service_Exthost.Effects.Documents.modelChanged(
          ~buffer=newBuffer, ~update, extHostClient, () =>
          Actions.ExtensionBufferUpdateQueued({triggerKey: triggerKey})
        ),
      )

    | BufferSaved(_) => (
        state,
        Isolinear.Effect.batch([gitRefreshEffect(state.scm)]),
      )

    | StatusBar(ContributedItemClicked({command, _})) => (
        state,
        Service_Exthost.Effects.Commands.executeContributedCommand(
          ~command,
          ~arguments=[],
          extHostClient,
        ),
      )

    | VimDirectoryChanged(path) => (state, changeWorkspaceEffect(path))

    | BufferEnter({id, filePath, _}) =>
      let eff =
        switch (filePath) {
        | Some(path) =>
          Feature_SCM.Effects.getOriginalUri(
            extHostClient, state.scm, path, uri =>
            Actions.GotOriginalUri({bufferId: id, uri})
          )

        | None => Isolinear.Effect.none
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

//    | ExtMessageReceived({severity, message, extensionId}) =>
//      let kind: Feature_Notification.kind =
//        switch (severity) {
//        | Exthost.Message.Ignore => Info
//        | Exthost.Message.Info => Info
//        | Exthost.Message.Warning => Warning
//        | Exthost.Message.Error => Error
//        };
//
//      (
//        state,
//        Feature_Notification.Effects.create(
//          ~kind,
//          ~source=?extensionId,
//          message,
//        )
//        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
//      );
//
    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
