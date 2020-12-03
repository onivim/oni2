/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;

let current = state => {
  let commands = CommandManager.current(state);

  Feature_Extensions.menus(state.extensions)
  |> Menu.Lookup.fromSchema(commands);
};
