module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Subscription = Core.Subscription;
module Log = (val Core.Log.withNamespace("Oni2.RipgrepSubscription"));

module Provider = {
  type action = Actions.t;
  type params = {onUpdate: Actions.menuItem => unit};

  let start = (~id, ~params, ~dispatch: _) => {
    Log.info("Starting DocumentSymbol subscription " ++ id);

    Revery.App.runOnMainThread(() => {
      let onUpdate = params.onUpdate;

      onUpdate(
        Actions.{
          category: None,
          name: "a",
          command: () =>
            Model.Actions.OpenFileByPath(
              "/Users/bryphe/revery/package.json",
              None,
              None,
            ),
          icon: None,
          highlight: [],
        },
      );

      onUpdate(
        Actions.{
          category: None,
          name: "b",
          command: () =>
            Model.Actions.OpenFileByPath(
              "/Users/bryphe/revery/package.json",
              None,
              None,
            ),
          icon: None,
          highlight: [],
        },
      );

      onUpdate(
        Actions.{
          category: None,
          name: "c",
          command: () =>
            Model.Actions.OpenFileByPath(
              "/Users/bryphe/revery/package.json",
              None,
              None,
            ),
          icon: None,
          highlight: [],
        },
      );
    });
  };

  let update = (~id, ~params, ~dispatch as _) => {
    Log.info("UPDATE: " ++ id);
  };

  let dispose = (~id) => {
    Log.info("Disposing: " ++ id);
  };
};

let create = (~id, ~onUpdate) =>
  Subscription.create(id, (module Provider), {onUpdate: onUpdate});
