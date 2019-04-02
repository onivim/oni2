/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

/* open Rench; */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let start = (setup: Core.Setup.t) => {
  ignore(setup);

  let _dispatch = ref(None);
  let dispatch = action => {
    switch (_dispatch^) {
    | None => ()
    | Some(v) => v(action)
    };
  };

  let stream =
    Isolinear.Stream.create(dispatch => _dispatch := Some(dispatch));

  ignore(_dispatch);
  let _ = dispatch;

  let position = (selectedItem, change, commands: list(Model.MenuCommand.t)) => {
    let nextIndex = selectedItem + change;
    nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
  };

  let selectItemEffect = command =>
    Isolinear.Effect.create(~name="menu.selectItem", command);

  let updateMenuCommands = (commands, state: Model.Menu.t) =>
    List.append(state.commands, commands);

  /* let updater = (_state, _action) => (_state, Isolinear.Effect.none); */

  let rec menuUpdater = (state: Model.Menu.t, action: Model.Actions.t) =>
    switch (action) {
    | MenuPosition(pos) => (
        {
          ...state,
          selectedItem: position(state.selectedItem, pos, state.commands),
        },
        Isolinear.Effect.none,
      )
    | MenuSearch(query) => (
        {
          ...state,
          searchQuery: query,
          commands: Model.Filter.menu(query, state.commands),
        },
        Isolinear.Effect.none,
      )
    /* | MenuOpen((menuType, commands)) => */
    /*   addCommands(commands, state.effects) */
    /*   |> (cmds => {...state, isOpen: true, menuType, commands: cmds}) */
    /*   |> (state => (state, Isolinear.Effect.none); */
    | MenuUpdate(update) => (
        {...state, commands: updateMenuCommands(update, state)},
        Isolinear.Effect.none,
      )
    | MenuClose => (
        {...state, isOpen: false, selectedItem: 0},
        Isolinear.Effect.none,
      )
    | MenuSelect =>
      let effect =
        List.nth(state.commands, state.selectedItem)
        |> (selected => selectItemEffect(selected.command));

      /* Also close menu */
      let (closeState, closeEffect) = menuUpdater(state, MenuClose);

      (closeState, Isolinear.Effect.batch([effect, closeEffect]));
    | _ => (state, Isolinear.Effect.none)
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
    let (menuState, menuEffect) = menuUpdater(state.menu, action);

    let state = {...state, menu: menuState};

    (state, menuEffect);
  };

  (updater, stream);
};
