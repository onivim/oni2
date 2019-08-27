/*
 * MenuStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Menu = Model.Menu;
module MenuJob = Model.MenuJob;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let position = (selectedItem, change, count) => {
    let nextSelectedItem = count > 0 ? (selectedItem + change) mod count : 0;
    nextSelectedItem >= 0 ? nextSelectedItem : count;
  };

  let menuOpenEffect = (menuConstructor, onQueryChangedEvent) =>
    Isolinear.Effect.create(~name="menu.construct", () => {
      let setItems = items => dispatch(Actions.MenuUpdate(items));
      let startTime = Revery.Time.getTime() |> Revery.Time.toSeconds;
      let setLoading = isLoading =>
        dispatch(Actions.MenuSetLoading(isLoading, startTime));

      let disposeFunction =
        menuConstructor(setItems, onQueryChangedEvent, setLoading);
      dispatch(Actions.MenuSetDispose(disposeFunction));
    });

  let queryChangedEffect = (evt, newQuery) =>
    Isolinear.Effect.create(~name="menu.queryChanged", () =>
      Rench.Event.dispatch(evt, newQuery)
    );

  let selectItemEffect = command =>
    Isolinear.Effect.createWithDispatch(~name="menu.selectItem", dispatch => {
      let action = command();
      dispatch(action);
    });

  let disposeMenuEffect = dispose =>
    Isolinear.Effect.create(~name="menu.dispose", dispose);

  let rec menuUpdater = (state: Menu.t, action: Actions.t) => {
    let filteredCommands =
      Core.Job.getCompletedWork(state.filterJob).uiFiltered;
    let filteredCommandsCount = filteredCommands |> Array.length;
    switch (action) {
    | MenuSetLoading(isLoading, time) => (
        {
          ...state,
          isLoading,
          loadingAnimation: Animation.start(time, state.loadingAnimation),
        },
        Isolinear.Effect.none,
      )
    | MenuPosition(index) => (
        {...state, selectedItem: index},
        Isolinear.Effect.none,
      )
    | MenuPreviousItem =>
      let itemToSelect =
        position(state.selectedItem, -1, filteredCommandsCount);
      let nextRowOffset =
        if (itemToSelect < state.selectedItem
            && itemToSelect < state.rowOffset
            && state.rowOffset
            - 1 > 0) {
          state.rowOffset - 1;
        } else if (itemToSelect > state.selectedItem) {
          filteredCommandsCount - state.maxRows;
        } else {
          state.rowOffset;
        };

      (
        {
          ...state,
          selectedItem: itemToSelect,
          rowOffset: nextRowOffset,
          scrollY: float_of_int(nextRowOffset * state.rowHeight),
        },
        Isolinear.Effect.none,
      );
    | MenuNextItem =>
      let itemToSelect =
        position(state.selectedItem, 1, filteredCommandsCount);
      let nextRowOffset =
        if (itemToSelect > state.selectedItem
            && itemToSelect
            - state.rowOffset > state.maxRows
            - 1) {
          state.rowOffset + 1;
        } else if (itemToSelect < state.selectedItem) {
          0;
        } else {
          state.rowOffset;
        };

      (
        {
          ...state,
          selectedItem: itemToSelect,
          rowOffset: nextRowOffset,
          scrollY: float_of_int(nextRowOffset * state.rowHeight),
        },
        Isolinear.Effect.none,
      );
    | MenuScroll(scrollDeltaY) =>
      let maxScrollY =
        filteredCommandsCount
        * state.rowHeight
        - state.maxRows
        * state.rowHeight;
      let newScrollY =
        float_of_int(
          min(
            maxScrollY,
            int_of_float(max(0., state.scrollY +. scrollDeltaY)),
          ),
        );

      ({...state, scrollY: newScrollY}, Isolinear.Effect.none);
    | MenuSearch(query) => (
        {
          ...state,
          searchQuery: query,
          filterJob:
            Core.Job.mapw(MenuJob.updateQuery(query), state.filterJob),
          selectedItem: 0,
        },
        queryChangedEffect(state.onQueryChanged, query),
      )
    | MenuOpen(menuConstructor) =>
      let state = Menu.create();
      (
        {...state, isOpen: true},
        menuOpenEffect(menuConstructor, state.onQueryChanged),
      );
    | MenuUpdate(update) =>
      let filterJob =
        Core.Job.mapw(MenuJob.addItems(update), state.filterJob);
      let selectedItem =
        position(state.selectedItem, 0, filteredCommandsCount);

      ({...state, filterJob, selectedItem}, Isolinear.Effect.none);
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
          isOpen: false,
          selectedItem: 0,
          isLoading: false,
          loadingAnimation: Animation.stop(state.loadingAnimation),
        },
        disposeMenuEffect(disposeFunction),
      );
    | MenuSelect =>
      let effect =
        switch (filteredCommands[state.selectedItem]) {
        | exception (Invalid_argument(_)) => Isolinear.Effect.none
        | v => selectItemEffect(v.command)
        };

      /* Also close menu */
      let (closeState, closeEffect) = menuUpdater(state, MenuClose);

      (closeState, Isolinear.Effect.batch([closeEffect, effect]));
    | _ => (state, Isolinear.Effect.none)
    };
  };

  let updateJob = (state: Model.State.t) =>
    if (Core.Job.isComplete(state.menu.filterJob)) {
      state;
    } else {
      {
        ...state,
        menu: {
          ...state.menu,
          filterJob: Core.Job.tick(state.menu.filterJob),
        },
      };
    };

  let updateAnimation = (deltaT: float, state: Model.State.t) =>
    if (state.menu.isLoading) {
      {
        ...state,
        menu: {
          ...state.menu,
          loadingAnimation:
            Animation.tick(deltaT, state.menu.loadingAnimation),
        },
      };
    } else {
      state;
    };

  let updater = (state: Model.State.t, action: Actions.t) =>
    switch (action) {
    | Actions.Tick({deltaTime, _}) =>
      let newState = state |> updateJob |> updateAnimation(deltaTime);

      (newState, Isolinear.Effect.none);
    | action =>
      let (menuState, menuEffect) = menuUpdater(state.menu, action);
      let state = {...state, menu: menuState};
      (state, menuEffect);
    };

  (updater, stream);
};
