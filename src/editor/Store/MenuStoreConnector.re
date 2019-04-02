/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

open Rench;

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let start = (setup: Core.Setup.t) => {

  let _dispatch = ref(None);
  let dispatch = action => {
    switch (_dispatch^) {
    | None => ()
    | Some(v) => v(action)
    };
  };

  let stream =
    Isolinear.Stream.create(dispatch => _dispatch := Some(dispatch));

let position = (selectedItem, change, commands: list(command)) => {
  let nextIndex = selectedItem + change;
  nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
};

    let selectItemEffect = (command) => Isolinear.Effect.create("menu.selectItem", () => command());

let updateMenuCommands = (commands, state: Menu.t) =>
  List.append(state.commands, commands);


  let updater = (state, action: Actions.t) =>
  switch (action) {
  | MenuPosition(pos) => ({
      ...state,
      selectedItem: position(state.selectedItem, pos, state.commands),
    }, Isolinear.Effect.none)
  | MenuSearch(query) => ({
      ...state,
      searchQuery: query,
      commands: Filter.menu(query, state.commands),
    }, Isolinear.Effect.none)
  /* | MenuOpen((menuType, commands)) => */
  /*   addCommands(commands, state.effects) */
  /*   |> (cmds => {...state, isOpen: true, menuType, commands: cmds}) */
  | MenuUpdate(update) => ({
      ...state,
      commands: updateMenuCommands(update, state),
    }, Isolinear.Effect.none),
  | MenuClose => ({...state, isOpen: false, menuType: Closed, selectedItem: 0}, Isolinear.Effect.none),
  | MenuSelect =>

    let effect = (List.nth(state.commands, state.selectedItem)
    |> (selected => selectItemEffect(selected.command));

    /* Also close menu */
    let (closeState, closeEffect) = updater(state, MenuClose);

    {closeState, Isolinear.Efffect.batch([effect, closeEffect])},
  | _ => (state, Isolinear.Effect.none),
  };

  (updater, stream);
};
