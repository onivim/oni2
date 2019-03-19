open Oni_Core.Types;
open UiMenu;

let create = (~effects: option(Effects.t)=?, ()) => {
  menuType: Closed,
  searchQuery: "",
  isOpen: false,
  commands: [],
  selectedItem: 0,
  effects,
};

let addEffects = (effects: Effects.t) =>
  Actions.MenuRegisterEffects(effects);

let position = (selectedItem, change, commands: list(command)) => {
  let nextIndex = selectedItem + change;
  nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
};

let addCommands = (factory: commandFactory, effects: option(Effects.t)) =>
  switch (effects) {
  | Some(e) => factory(e)
  | None => []
  };

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
  | MenuOpen((menu, commands)) =>
    /**
     If the command factory for a module trying to open a menu returns
     0 options/commands for the menu do not open an empty menu.
     this is a safeguard *IF* a command factory function fails
   */
    addCommands(commands, state.effects)
    |> (
      cmds =>
        List.length(cmds) > 0 ?
          {...state, isOpen: true, menuType: menu, commands: cmds} : state
    )
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
