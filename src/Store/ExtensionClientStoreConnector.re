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
          () => {
            Log.debug("terminating exthost");
            Exthost.Client.terminate(extHostClient);
          },
        ),
      )
    );

  let updater = (state: State.t, action: Actions.t) =>
    switch (action) {
    | Init => (
        state,
        Isolinear.Effect.batch([
          registerQuitCleanupEffect,
          discoveredExtensionsEffect(extensions),
        ]),
      )

    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
