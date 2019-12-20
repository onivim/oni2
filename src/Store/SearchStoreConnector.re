open Oni_Model;
open Actions;

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

    | ActivityBar(ActivityBar.SearchClick) when state.searchPane != None => (
        {...state, searchPane: None} |> FocusManager.pop(Search),
        Isolinear.Effect.none,
      )

    | ActivityBar(ActivityBar.SearchClick) when state.searchPane == None => (
        {...state, searchPane: Some(Search.initial)}
        |> FocusManager.push(Search),
        Isolinear.Effect.none,
      )

    | SearchShow => (
        {...state, searchPane: Some(Search.initial)}
        |> FocusManager.push(Search),
        Isolinear.Effect.none,
      )

    | SearchHide => (
        {...state, searchPane: None} |> FocusManager.pop(Search),
        Isolinear.Effect.none,
      )

    | SearchInputClicked(_) =>
      switch (state.searchPane) {
      | Some(searchPane) => (
          {...state, searchPane: Some(searchUpdater(searchPane, action))}
          |> FocusManager.push(Search),
          Isolinear.Effect.none,
        )
      | None => (state, Isolinear.Effect.none)
      }

    | SearchHotkey =>
      switch (state.searchPane) {
      | Some(_) => (
          state |> FocusManager.push(Search),
          Isolinear.Effect.none,
        )
      | None => (
          {...state, searchPane: Some(Search.initial)}
          |> FocusManager.push(Search),
          Isolinear.Effect.none,
        )
      }

    | _ =>
      switch (state.searchPane) {
      | Some(searchPane) => (
          {...state, searchPane: Some(searchUpdater(searchPane, action))},
          Isolinear.Effect.none,
        )
      | None => (state, Isolinear.Effect.none)
      }
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
      ~onUpdate=items => {dispatch(SearchUpdate(items))},
      ~onCompleted=() => SearchComplete,
    );
  };

  let updater = (state: State.t) => {
    switch (state.searchPane) {
    | None
    | Some({query: "", _}) => []

    | Some({query, _}) => [search(query)]
    };
  };

  (updater, stream);
};
