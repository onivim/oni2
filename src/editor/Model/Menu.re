open Oni_Core;
open Types;
open UiMenu;

type effectsT = Effects.t(Actions.t);

let create = (~effects: option(effectsT)=?, ()) => {
  menuType: Closed,
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
  effects,
};

let addEffects = (effects: effectsT) => Actions.MenuRegisterEffects(effects);

let position = (selectedItem, change, commands: list(command)) => {
  let nextIndex = selectedItem + change;
  nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
};

let addCommands =
    (factory: commandFactory(Actions.t), effects: option(effectsT)) =>
  switch (effects) {
  | Some(e) => factory(e)
  | None => []
  };

let updateMenuCommands = ((mType, commands), state: UiMenu.t(Actions.t)) =>
  mType == state.menuType ? List.append(state.commands, commands) : commands;

let reduce = (state, action: Actions.t) =>
  switch (action) {
  | MenuRegisterEffects(effects) => {...state, effects: Some(effects)}
  | MenuPosition(pos) => {
      ...state,
      selectedItem: position(state.selectedItem, pos, state.commands),
    }
  | MenuSearch(query) => {
      ...state,
      searchQuery: query,
      commands: Filter.menu(query, state.commands),
    }
  | MenuOpen((menuType, commands)) =>
    addCommands(commands, state.effects)
    |> (cmds => {...state, isOpen: true, menuType, commands: cmds})
  | MenuUpdate(update) => {
      ...state,
      commands: updateMenuCommands(update, state),
    }
  | MenuClose => {...state, isOpen: false, menuType: Closed, selectedItem: 0}
  | MenuSelect =>
    /**
      TODO: Refactor this to middleware so this action is handled like a redux side-effect
      as this makes the reducer impure
     */
    List.nth(state.commands, state.selectedItem)
    |> (selected => selected.command());
    {...state, isOpen: false, menuType: Closed};
  | _ => state
  };
