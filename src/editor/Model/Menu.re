open Oni_Core.Types;
open UiMenu;

let create = (~effects: option(Effects.t)=?, ()) => {
  menuType: Closed,
  isOpen: false,
  commands: [],
  selectedItem: 0,
  effects,
};

let addEffects = (effects: Effects.t) =>
  Actions.MenuRegisterEffects(effects);

let position = (selectedItem, change, commands: list(command)) =>
  selectedItem + change >= List.length(commands) ? 0 : selectedItem + change;

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
  | MenuOpen((menu, commands)) => {
      ...state,
      isOpen: true,
      menuType: menu,
      commands: addCommands(commands, state.effects),
    }
  | MenuClose => {...state, isOpen: false, menuType: Closed}
  | MenuSelect =>
    /**
    TODO: Refactor this to middleware so this action is handled like a redux side-effect
    as this makes the reducer impure
   */
    let selected = List.nth(state.commands, state.selectedItem);
    selected.command();
    {...state, isOpen: false};
  | _ => state
  };
