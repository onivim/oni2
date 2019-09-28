/*
 * InputStoreConnector.re
 *
 * This module connects external user input to the store.
 */

module Model = Oni_Model;

let start =
    (
      getState: unit => Model.State.t,
      window: Revery.Window.t,
    ) => {

  // Noop
  ();
};
