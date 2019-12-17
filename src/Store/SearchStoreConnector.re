module Core = Oni_Core;
module Model = Oni_Model;

open Model.Actions;

let start = () => {
  let (stream, _dispatch) = Isolinear.Stream.create();

  let searchUpdater = (state: Model.Search.t, action) => {
    switch (action) {
    | SearchInput(text, cursorPosition) => {
        ...state,
        queryInput: text,
        cursorPosition,
      }

    | SearchStart => {...state, query: state.queryInput, hits: []}

    | SearchUpdate(items) => {...state, hits: state.hits @ items}

    // | SearchComplete

    // | SearchSelectResult(match) =>
    //   print_endline("!! SELECT: " ++ match.file);
    //   state;

    | _ => state
    };
  };

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Tick(_) => (state, Isolinear.Effect.none)
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

  let updater = (state: Model.State.t) => {
    switch (state.searchPane) {
    | {query: "", _} => []
    | {query, _} => [search(query)]
    };
  };

  (updater, stream);
};
