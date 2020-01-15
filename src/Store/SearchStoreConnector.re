open Oni_Model;
open Actions;

module InputModel = Oni_Components.InputModel;

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let searchUpdater = (state: Search.t, action) => {
    switch (action) {
    | SearchInput(key) =>
      let Search.{queryInput, cursorPosition, _} = state;

      switch (key) {
      | "<CR>" => {...state, query: state.queryInput, hits: []}

      | _ =>
        let (queryInput, cursorPosition) =
          InputModel.handleInput(~text=queryInput, ~cursorPosition, key);
        {...state, queryInput, cursorPosition};
      };

    | SearchInputClicked(cursorPosition) => {...state, cursorPosition}

    | SearchUpdate(items) => {...state, hits: state.hits @ items}

    | _ => state
    };
  };

  let updater = (state: State.t, action) => {
    switch (action) {
    | Tick(_) => (state, Isolinear.Effect.none)
    | SearchInputClicked(_) => (
        {...state, searchPane: searchUpdater(state.searchPane, action)}
        |> FocusManager.push(Search),
        Isolinear.Effect.none,
      )
    | _ => (
        {...state, searchPane: searchUpdater(state.searchPane, action)},
        Isolinear.Effect.none,
      )
    };
  };

  (updater, stream);
};

// SUBSCRIPTIONS

let subscriptions = ripgrep => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let search = query => {
    let directory = Rench.Environment.getWorkingDirectory();

    SearchSubscription.create(
      ~id="workspace-search",
      ~query,
      ~directory,
      ~ripgrep,
      ~onUpdate=items => dispatch(SearchUpdate(items)),
      ~onCompleted=() => SearchComplete,
    );
  };

  let updater = (state: State.t) => {
    switch (state.searchPane) {
    | {query: "", _} => []
    | {query, _} => [search(query)]
    };
  };

  (updater, stream);
};
