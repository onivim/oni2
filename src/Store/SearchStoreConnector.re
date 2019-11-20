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

    | SearchStart => {...state, query: state.queryInput,hits: []}

    | SearchUpdate(items) => {...state, hits: state.hits @ items}

    // | SearchComplete

    | SearchSelectResult(match) =>
      print_endline("!! SELECT: " ++ match.file);
      state

    | _ => state
    };
  };

  let updater = (state: Model.State.t, action) => {
    let searchPane =
      switch (action) {
      | SearchShow => Some(Model.Search.initial)

      | SearchHide => None

      | _ =>
        switch (state.searchPane) {
        | Some(searchPane) => Some(searchUpdater(searchPane, action))
        | None => None
        }
      };

    ({...state, searchPane}, Isolinear.Effect.none);
  };

  (updater, stream);
};
