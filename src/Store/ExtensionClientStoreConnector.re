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

module Diagnostic = Feature_LanguageSupport.Diagnostic;
module LanguageFeatures = Feature_LanguageSupport.LanguageFeatures;

let start = (extensions, extHostClient: Exthost.Client.t) => {
  let discoveredExtensionsEffect = extensions =>
    Isolinear.Effect.createWithDispatch(
      ~name="exthost.discoverExtensions", dispatch =>
      dispatch(
        Actions.Extensions(Feature_Extensions.Msg.discovered(extensions)),
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

  let updater = (state: State.t, action: Actions.t) =>
    switch (action) {
    | Init => (
        state,
        Isolinear.Effect.batch([
          registerQuitCleanupEffect,
          discoveredExtensionsEffect(extensions),
        ]),
      )

    | Buffers(
        Feature_Buffers.Update({update, newBuffer, triggerKey, oldBuffer}),
      ) => (
        state,
        Service_Exthost.Effects.Documents.modelChanged(
          ~previousBuffer=oldBuffer,
          ~buffer=newBuffer,
          ~update,
          extHostClient,
          () =>
          Actions.ExtensionBufferUpdateQueued({triggerKey: triggerKey})
        ),
      )

    | Buffers(Feature_Buffers.Saved(bufferId)) =>
      let effect =
        state.buffers
        |> Feature_Buffers.get(bufferId)
        |> Option.map(buffer => {
             Service_Exthost.Effects.FileSystemEventService.onFileEvent(
               ~events=
                 Exthost.Files.FileSystemEvents.{
                   created: [],
                   deleted: [],
                   changed: [buffer |> Oni_Core.Buffer.getUri],
                 },
               extHostClient,
             )
           })
        |> Option.value(~default=Isolinear.Effect.none);

      (state, effect);

    | DirectoryChanged(path) => (state, changeWorkspaceEffect(path))

    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
