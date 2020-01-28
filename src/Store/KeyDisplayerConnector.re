/*
 * KeyDisplayerConnector.re
 *
 * This implements an updater (reducer + side effects) for the KeyDisplayer
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module KeyDisplayer = Model.KeyDisplayer;

let start = () => {
  let reducer = (model: Model.KeyDisplayer.t, action: Actions.t) => {
    switch (action) {
    | Actions.EnableKeyDisplayer => KeyDisplayer.enable(model)
    | Actions.DisableKeyDisplayer => KeyDisplayer.initial
    | Actions.NotifyKeyPressed(time, key)
        when model.isEnabled && Oni_Input.Filter.filter(key) =>
      KeyDisplayer.add(time, key, model)
    | _ => model
    };
  };

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Tick(_) => (state, Isolinear.Effect.none)
    | _ =>
      let keyDisplayer = reducer(state.keyDisplayer, action);
      let state = {...state, keyDisplayer};
      (state, Isolinear.Effect.none);
    };
  };

  updater;
};
