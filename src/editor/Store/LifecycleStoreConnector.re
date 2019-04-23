/*
 * LifecycleStoreConnector.re
 *
 * This manages side-effects related to the lifecycle.
 * - Handling quit cleanup
 */

open Revery;

module Core = Oni_Core;
module Model = Oni_Model;

Printexc.record_backtrace(true);

let start = () => {
  let quitEffect = (handlers) =>
    Isolinear.Effect.create(~name="lifecycle.quit", () => {
        print_endline ("Quitting...");
        List.iter((h) => h(), handlers);
        print_endline ("Quit!");
        App.quit(0);
    });

  let updater = (state: Model.State.t, action) => {
  switch (action) {
  | Model.Actions.Quit => (state, quitEffect(state.lifecycle.onQuitFunctions))
  | _ => (state, Isolinear.Effect.none)
  }
  };

  updater;
};
