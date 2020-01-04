/*
 * CompletionStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for completion
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Model;
open Actions;

module Option = Utility.Option;
module VimEx = Utility.VimEx;

module Log = (val Log.withNamespace("Oni2.CompletionStore"));

type lastCompletionMeet = {
  completionMeetBufferId: int,
  completionMeetLine: Index.t,
  completionMeetColumn: Index.t,
};

let defaultMeet = {
  completionMeetBufferId: (-1),
  completionMeetLine: Index.fromZeroBased(-1),
  completionMeetColumn: Index.fromZeroBased(-1),
};

let lastMeet = ref(defaultMeet);

let lastBase = ref("");

let equals = (~line, ~column, ~bufferId, oldMeet) => {
  oldMeet.completionMeetLine == line
  && oldMeet.completionMeetColumn == column
  && oldMeet.completionMeetBufferId == bufferId;
};

let applyCompletion = (state: State.t) =>
  Isolinear.Effect.createWithDispatch(~name="vim.applyCompletion", dispatch => {
    let maybeFocused =
      Option.map(
        i => state.completions.filtered[i],
        state.completions.focused,
      );

    let maybeMeetPosition =
      state.completions
      |> Completions.getMeet
      |> Option.map(CompletionMeet.getLocation);

    switch (maybeFocused, maybeMeetPosition) {
    | (Some(completion), Some({column, _})) =>
      let cursor = Vim.Cursor.get();
      let delta =
        Index.toZeroBased(cursor.column) - Index.toZeroBased(column);

      Log.infof(m =>
        m(
          "Completing at cursor position: %s | meet: %s",
          Index.show(cursor.column),
          Index.show(column),
        )
      );

      let _: list(Vim.Cursor.t) = VimEx.repeatInput(delta, "<BS>");
      let cursors = VimEx.inputString(completion.item.label);

      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getId)
      |> Option.iter(id => {dispatch(EditorCursorMove(id, cursors))});
    | _ => ()
    };
  });

let start = () => {
  let checkCompletionMeet = (state: State.t) =>
    Isolinear.Effect.createWithDispatch(~name="completion.checkMeet", dispatch => {
      let editor =
        state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

      editor
      |> Option.iter(ed => {
           let bufferId = ed.bufferId;
           let bufferOpt = Buffers.getBuffer(bufferId, state.buffers);
           switch (bufferOpt) {
           | None => ()
           | Some(buffer) =>
             let cursorPosition = Editor.getPrimaryCursor(ed);
             let meetOpt =
               CompletionMeet.createFromBufferLocation(
                 ~location=cursorPosition,
                 buffer,
               );
             switch (meetOpt) {
             | None =>
               lastMeet := defaultMeet;
               dispatch(Actions.CompletionEnd);
             | Some(meet) =>
               open CompletionMeet;
               let {line, column}: Location.t = getLocation(meet);
               let base = getBase(meet);
               // Check if our 'meet' position has changed
               let newMeet = {
                 completionMeetBufferId: bufferId,
                 completionMeetLine: line,
                 completionMeetColumn: column,
               };
               if (!equals(~line, ~column, ~bufferId, lastMeet^)) {
                 Log.infof(m =>
                   m(
                     "New completion meet: %s",
                     CompletionMeet.toString(meet),
                   )
                 );
                 dispatch(CompletionStart(meet));
                 dispatch(CompletionBaseChanged(base));
               } else if (!String.equal(base, lastBase^)) {
                 // If we're at the same position... but our base is different...
                 // fire a base change
                 lastBase := base;
                 dispatch(CompletionBaseChanged(base));
                 Log.info("New completion base: " ++ base);
               };
               lastMeet := newMeet;
             };
           };
         });
    });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Command("acceptSelectedSuggestion") => (state, applyCompletion(state))
    | ChangeMode(mode) when mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | EditorCursorMove(_) when state.mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
