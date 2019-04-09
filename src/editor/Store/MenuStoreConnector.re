/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

open Oni_Model;
module Core = Oni_Core;

module Extensions = Oni_Extensions;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let position = (selectedItem, change, commands) => {
    let nextIndex = selectedItem + change;
    nextIndex >= List.length(commands) || nextIndex < 0 ? 0 : nextIndex;
  };

  let menuOpenEffect = menuConstructor =>
    Isolinear.Effect.create(~name="menu.construct", () => {
      let setItems = items => dispatch(Actions.MenuUpdate(items));

      let disposeFunction = menuConstructor(setItems);
      dispatch(Actions.MenuSetDispose(disposeFunction));
    });

  let selectItemEffect = command =>
    Isolinear.Effect.createWithDispatch(~name="menu.selectItem", dispatch => {
      let action = command();
      dispatch(action);
    });

  let updateMenuCommands = (commands, state: Menu.t(State.t)) =>
    List.append(state.commands, commands);

  let disposeMenuEffect = dispose =>
    Isolinear.Effect.create(~name="menu.dispose", dispose);

  let rec menuUpdater = (state: Menu.t(State.t), action: Actions.t(State.t)) =>
    switch (action) {
    | MenuPosition(index) => (
        {...state, selectedItem: index},
        Isolinear.Effect.none,
      )
    | MenuPreviousItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, -1, state.commands),
        },
        Isolinear.Effect.none,
      )
    | MenuNextItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, 1, state.commands),
        },
        Isolinear.Effect.none,
      )
    | MenuSearch(query) => (
        {
          ...state,
          searchQuery: query,
          commands: Filter.menu(query, state.commands),
        },
        Isolinear.Effect.none,
      )
    | MenuOpen(menuConstructor) => (
        {...state, isOpen: true, commands: []},
        menuOpenEffect(menuConstructor),
      )
    | MenuUpdate(update) => (
        {...state, commands: updateMenuCommands(update, state)},
        Isolinear.Effect.none,
      )
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
        List.nth(state.commands, state.selectedItem)
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
