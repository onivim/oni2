/*
 * ExtensionClientStoreConnector.re
 *
 * This connects the extension client to the store:
 * - Converts extension host notifications into ACTIONS
 * - Calls appropriate APIs on extension host based on ACTIONS
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

open Oni_Extensions;

let start = (extensions, setup: Core.Setup.t) => {
  let (stream, dispatch) =
    Isolinear.Stream.create();

  let onExtHostClosed = () => print_endline("ext host closed");

  let extensionInfo =
    extensions
    |> List.map(ext =>
         Extensions.ExtensionHostInitData.ExtensionInfo.ofScannedExtension(
           ext,
         )
       );

  let onMessage = (scope, method, args) => {
    switch (scope, method, args) {
    | (
        "MainThreadStatusBar",
        "$setEntry",
        [
          `Int(id),
          _,
          `String(text),
          _,
          _,
          _,
          `Int(alignment),
          `Int(priority),
        ],
      ) =>
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
      Ok(None);
    | _ => Ok(None)
    };
  };

  let initData = ExtensionHostInitData.create(~extensions=extensionInfo, ());
  let extHostClient =
    Extensions.ExtensionHostClient.start(
      ~initData,
      ~onClosed=onExtHostClosed,
      ~onMessage,
      setup,
    );

  let pumpEffect =
    Isolinear.Effect.create(~name="exthost.pump", () =>
      Extensions.ExtensionHostClient.pump(extHostClient)
    );

  let updater = (state, action) =>
    switch (action) {
    | Model.Actions.Tick => (state, pumpEffect)
    | _ => (state, Isolinear.Effect.none)
    };

  (updater, stream);
};
