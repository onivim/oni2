/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let position =
      (selectedItem, change, commands: list(Model.Actions.menuCommand)) => {
    let nextIndex = selectedItem + change;
    nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
  };

  let menuOpenEffect = menuConstructor =>
    Isolinear.Effect.create(~name="menu.construct", () => {
      let setItems = items => dispatch(Model.Actions.MenuUpdate(items));

      let disposeFunction = menuConstructor(setItems);
      dispatch(Model.Actions.MenuSetDispose(disposeFunction));
    });

  let selectItemEffect = command =>
    Isolinear.Effect.createWithDispatch(~name="menu.selectItem", dispatch => {
      let action = command();
      dispatch(action);
    });

  let disposeMenuEffect = dispose =>
    Isolinear.Effect.create(~name="menu.dispose", dispose);

  let rec menuUpdater = (state: Model.Menu.t, action: Model.Actions.t) =>
    switch (action) {
    | MenuPosition(index) => (
        {...state, selectedItem: index},
        Isolinear.Effect.none,
      )
    | MenuPreviousItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, -1, state.filteredCommands),
        },
        Isolinear.Effect.none,
      )
    | MenuNextItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, 1, state.filteredCommands),
        },
        Isolinear.Effect.none,
      )
    | MenuSearch(query) => (
        {
          ...state,
          searchQuery: query,
          filteredCommands: Model.Filter.menu(query, state.commands),
        },
        Isolinear.Effect.none,
      )
    | MenuOpen(menuConstructor) => (
        {...state, isOpen: true, commands: []},
        menuOpenEffect(menuConstructor),
      )
    | MenuUpdate(update) =>
      let commands = List.append(state.commands, update);
      let filteredCommands = Model.Filter.menu(state.searchQuery, commands);

      ({...state, commands, filteredCommands}, Isolinear.Effect.none);
    | MenuSetDispose(dispose) => (
        {...state, dispose},
        Isolinear.Effect.none,
      )
    | MenuClose =>
      let disposeFunction = state.dispose;
      (
        {...state, commands: [], isOpen: false, selectedItem: 0},
        disposeMenuEffect(disposeFunction),
      );
    | MenuSelect =>
      let effect =
        List.nth(state.filteredCommands, state.selectedItem)
        |> (selected => selectItemEffect(selected.command));

      /* Also close menu */
      let (closeState, closeEffect) = menuUpdater(state, MenuClose);

      (closeState, Isolinear.Effect.batch([effect, closeEffect]));
    | _ => (state, Isolinear.Effect.none)
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    if (action === Model.Actions.Tick) {
      (state, Isolinear.Effect.none);
    } else {
      let (menuState, menuEffect) = menuUpdater(state.menu, action);
      let state = {...state, menu: menuState};
      (state, menuEffect);
    };

  (updater, stream);
};
