/*
 * TextSynchronization.re
 */
open EditorCoreTypes;
module Core = Oni_Core;

module Model = Oni_Model;

open Types;

let _getTextForVimBuffer = () => {
  let buffer = Vim.Buffer.getCurrent();
  let count = Vim.Buffer.getLineCount(buffer);

  let i = ref(1);
  let lines = ref([]);
  while (i^ <= count) {
    lines := [Vim.Buffer.getLine(buffer, Index.fromOneBased(i^)), ...lines^];
    incr(i);
  };
  "fulltext:" ++ String.concat("|", lines^ |> List.rev) ++ "|";
};

let _getTextForOnivimBuffer = state => {
  Model.Selectors.getActiveBuffer(state)
  |> Option.map(Core.Buffer.getLines)
  |> Option.map(Array.to_list)
  |> Option.map(String.concat("|"))
  |> Option.map(s => "fulltext:" ++ s ++ "|")
  |> Option.value(~default="(Empty)");
};

let _getTextFromExtHost = state => {
  Model.State.(
    switch ((state.notifications :> list(Feature_Notification.notification))) {
    | [hd, ..._] => hd.message
    | [] => "(Empty)"
    }
  );
};

let validateTextIsSynchronized =
    (
      ~expectedText=None,
      dispatch: dispatchFunction,
      wait: waitForState,
      ~description,
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

      let onivimBuffer = _getTextForOnivimBuffer(state);
      let extHostBuffer = _getTextFromExtHost(state);
      let vimBuffer = _getTextForVimBuffer();

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
