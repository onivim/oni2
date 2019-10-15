/*
 * HoverStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Hover UX
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Menu = Model.Menu;
module MenuJob = Model.MenuJob;

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let updater = (state: Model.State.t, action: Actions.t) =>
    switch (action) {
    | Actions.Tick({deltaTime, _}) =>
      let newState = state |> updateJob |> updateAnimation(deltaTime);

      (newState, Isolinear.Effect.none);
    | action =>
      let (menuState, menuEffect) = menuUpdater(state.menu, action);
      let state = {...state, menu: menuState};
      (state, menuEffect);
    };

  (updater, stream);
};
