/*
 * KeyDisplayerConnector.re
 *
 * This implements an updater (reducer + side effects) for the KeyDisplayer
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module KeyDisplayer = Model.KeyDisplayer;

let start = getTime => {
  let reducer = (keyDisplayer: Model.KeyDisplayer.t, action: Actions.t) => {
    switch (action) {
    | Actions.EnableKeyDisplayer =>
      KeyDisplayer.setEnabled(true, keyDisplayer)
    | Actions.DisableKeyDisplayer =>
      KeyDisplayer.setEnabled(false, keyDisplayer)
    | Actions.NotifyKeyPressed(time, key)
        when KeyDisplayer.getEnabled(keyDisplayer) =>
      KeyDisplayer.add(time, key, keyDisplayer)
    | _ => keyDisplayer
    };
  };

  let updater = (state: Model.State.t, action: Actions.t) =>
    switch (action) {
    | Actions.Tick(_) =>
      if (KeyDisplayer.getActive(state.keyDisplayer)) {
        let keyDisplayer = KeyDisplayer.update(getTime(), state.keyDisplayer);
        let newState = {...state, keyDisplayer};
        (newState, Isolinear.Effect.none);
      } else {
        (state, Isolinear.Effect.none);
      }
    | action =>
      let keyDisplayer = reducer(state.keyDisplayer, action);
      let state = {...state, keyDisplayer};
      (state, Isolinear.Effect.none);
    };
  updater;
};
