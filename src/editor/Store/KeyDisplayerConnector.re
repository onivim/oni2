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
    | Actions.NotifyKeyPressed(time, key) =>
      KeyDisplayer.add(time, key, keyDisplayer)
    | _ => keyDisplayer
    };
  };

  let updater = (state: Model.State.t, action: Actions.t) =>
    if (action === Actions.Tick) {
      if (!state.keyDisplayer.active) {
        (state, Isolinear.Effect.none);
      } else {
        let keyDisplayer = KeyDisplayer.update(getTime(), state.keyDisplayer);
        let newState = {...state, keyDisplayer};

        (newState, Isolinear.Effect.none);
      };
    } else {
      let keyDisplayer = reducer(state.keyDisplayer, action);
      let state = {...state, keyDisplayer};
      (state, Isolinear.Effect.none);
    };
  updater;
};
