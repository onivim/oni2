/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;
module MenuJob = Model.MenuJob;

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

  let rec menuUpdater = (state: Model.Menu.t, action: Model.Actions.t) => {
    let filteredCommands = Core.Job.getCompletedWork(state.filterJob);
    switch (action) {
    | MenuPosition(index) => (
        {...state, selectedItem: index},
        Isolinear.Effect.none,
      )
    | MenuPreviousItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, -1, filteredCommands),
        },
        Isolinear.Effect.none,
      )
    | MenuNextItem => (
        {
          ...state,
          selectedItem: position(state.selectedItem, 1, filteredCommands),
        },
        Isolinear.Effect.none,
      )
    | MenuSearch(query) => (
        {
          ...state,
          searchQuery: query,
          filterJob:
            Core.Job.map(MenuJob.updateQuery(query), state.filterJob),
        },
        Isolinear.Effect.none,
      )
    | MenuOpen(menuConstructor) => (
        {...state, isOpen: true, commands: []},
        menuOpenEffect(menuConstructor),
      )
    | MenuUpdate(update) =>
      let commands = List.append(state.commands, update);
      let filterJob =
        Core.Job.map(MenuJob.addItems(update), state.filterJob);

      ({...state, commands, filterJob}, Isolinear.Effect.none);
    | MenuSetDispose(dispose) => (
        {...state, dispose},
        Isolinear.Effect.none,
      )
    | MenuClose =>
      let disposeFunction = state.dispose;
      (
        {
          ...state,
          filterJob: MenuJob.default,
          commands: [],
          isOpen: false,
          selectedItem: 0,
        },
        disposeMenuEffect(disposeFunction),
      );
    | MenuSelect =>
      let effect =
        List.nth(filteredCommands, state.selectedItem)
        |> (selected => selectItemEffect(selected.command));

      /* Also close menu */
      let (closeState, closeEffect) = menuUpdater(state, MenuClose);

      (closeState, Isolinear.Effect.batch([effect, closeEffect]));
    | _ => (state, Isolinear.Effect.none)
    };
  };

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    if (action === Model.Actions.Tick) {
      if (Core.Job.isComplete(state.menu.filterJob)) {
        (state, Isolinear.Effect.none);
      } else {
        print_endline("Work is not done!");
        let newState = {
          ...state,
          menu: {
            ...state.menu,
            filterJob: Core.Job.tick(state.menu.filterJob),
          },
        };
        print_endline(
          "Latest status: " ++ Core.Job.show(newState.menu.filterJob),
        );
        (newState, Isolinear.Effect.none);
      };
    } else {
      let (menuState, menuEffect) = menuUpdater(state.menu, action);
      let state = {...state, menu: menuState};
      (state, menuEffect);
    };

  (updater, stream);
};
