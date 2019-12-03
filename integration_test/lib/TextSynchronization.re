/*
 * TextSynchronization.re
 */
module Core = Oni_Core;
module Option = Core.Utility.Option;

module Model = Oni_Model;

type dispatchFunction = Model.Actions.t => unit;
type waiter = Model.State.t => bool;
type waitForState = (~name: string, ~timeout: float=?, waiter) => unit;

let getTextForVimBuffer = () => {
  let buffer = Vim.Buffer.getCurrent();
  let count = Vim.Buffer.getLineCount(buffer);

  let i = ref(1);
  let lines = ref([]);
  while (i^ <= count) {
    lines := [Vim.Buffer.getLine(buffer, i^), ...lines^];
    incr(i);
  };
  "fulltext:" ++ String.concat("|", lines^ |> List.rev) ++ "|";
};

let getTextForOnivimBuffer = state => {
  Some(state)
  |> Option.bind(Model.Selectors.getActiveBuffer)
  |> Option.map(Model.Buffer.getLines)
  |> Option.map(Array.to_list)
  |> Option.map(String.concat("|"))
  |> Option.map(s => "fulltext:" ++ s ++ "|")
  |> Option.value(~default="(Empty)");
};

let getTextFromExtHost = state => {
  Model.State.(
    switch (state.notifications) {
    | [hd, ..._] => hd.message
    | [] => "(Empty)"
    }
  );
};

let validateTextIsSynchronized =
    (
      ~expectedText=None,
      ~description,
      dispatch: dispatchFunction,
      wait: waitForState,
    ) => {
  wait(
    ~name=description ++ ": validate text is synchronized",
    ~timeout=30.0,
    (state: Model.State.t) => {
      // Request the latest buffer text from the extension
      dispatch(
        Model.Actions.CommandExecuteContributed(
          "developer.oni.getBufferText",
        ),
      );

      let onivimBuffer = getTextForOnivimBuffer(state);
      let extHostBuffer = getTextFromExtHost(state);
      let vimBuffer = getTextForVimBuffer();

      print_endline("Onivim buffer: " ++ onivimBuffer);
      print_endline("Exthost buffer: " ++ extHostBuffer);
      print_endline("Vim buffer: " ++ vimBuffer);
      let expectedEqual =
        switch (expectedText) {
        | None => true
        | Some(str) => String.equal("fulltext:" ++ str, onivimBuffer)
        };
      expectedEqual
      && String.equal(onivimBuffer, extHostBuffer)
      && String.equal(onivimBuffer, vimBuffer);
    },
  );
};
