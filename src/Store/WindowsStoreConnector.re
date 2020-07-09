/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;
open Model.Actions;

module OptionEx = Core.Utility.OptionEx;

let start = () => {
  let windowUpdater = (state: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | OpenFileByPath(_) => FocusManager.push(Editor, state)

    | _ => state
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => (
    windowUpdater(state, action),
    Isolinear.Effect.none,
  );

  updater;
};
