/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 */

open Oni_Model;

let start = (~quit, ~raiseWindow) => {
  let quitAllEffect = (state: State.t, force) => {
    let handlers = state.lifecycle.onQuitFunctions;

    let anyModified = Buffers.anyModified(state.buffers);
    let canClose = force || !anyModified;

    Isolinear.Effect.createWithDispatch(~name="lifecycle.quitAll", dispatch =>
      if (canClose) {
        dispatch(Actions.ReallyQuitting);
        List.iter(h => h(), handlers);
        quit(0);
      }
    );
  };

  let internalWindowRaiseEffect =
    Isolinear.Effect.create(~name="window.raise", () => raiseWindow());

  let updater = (state: State.t, action) => {
    switch (action) {
    | Actions.QuitBuffer(buffer, force) =>
      switch (Feature_Layout.closeBuffer(~force, buffer, state.layout)) {
      | Some(layout) => ({...state, layout}, Isolinear.Effect.none)
      | None => (state, quitAllEffect(state, force))
      }

    | Actions.Quit(force) => (state, quitAllEffect(state, force))

    | WindowCloseBlocked => (
        {...state, modal: Some(Feature_Modals.unsavedBuffersWarning)},
        internalWindowRaiseEffect,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
