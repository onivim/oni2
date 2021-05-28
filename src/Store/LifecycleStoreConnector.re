/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 *
 * Quitting always goes through revery, but may not go through the Quit action.
 * - if we might want to quit, we emit Quit, which might call revery's quit(0)
 * - if the application is explicitly quit, that goes directly to revery's quit(0)
 *
 * The onBeforeQuit handler we install in revery always prevents immediate quit, and
 * instead emits ReallyQuitting. Our ReallyQuitting effect invokes all of our own
 * `onQuitFunctions`, and then exits the process.
 */

open Oni_Model;

let start = (~quit, ~raiseWindow) => {
  let quitAllEffect = (state: State.t, force) => {
    let anyModified = Feature_Buffers.anyModified(state.buffers);
    let canClose = force || !anyModified;
    Isolinear.Effect.create(~name="lifecycle.quitAll", () =>
      if (canClose) {
        quit(0);
      }
    );
  };

  let reallyQuitEffect = (state: State.t) => {
    let handlers = state.lifecycle.onQuitFunctions;
    Isolinear.Effect.create(~name="lifecycle.reallyQuit", () => {
      List.iter(h => h(), handlers);
      exit(0);
    });
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

    | Actions.ReallyQuitting => (state, reallyQuitEffect(state))

    | WindowCloseBlocked => (
        {...state, modal: Some(Feature_Modals.unsavedBuffersWarning)},
        internalWindowRaiseEffect,
      )

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
