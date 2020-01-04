/*
 * CompletionStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for completion
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Actions;

module Zed_utf8 = ZedBundled;

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

let equals = (~line, ~column, ~bufferId, oldMeet: lastCompletionMeet) => {
  oldMeet.completionMeetLine == line
  && oldMeet.completionMeetColumn == column
  && oldMeet.completionMeetBufferId == bufferId;
};

let applyCompletion = (state: State.t) =>
  Isolinear.Effect.createWithDispatch(~name="vim.applyCompletion", dispatch => {
    let completions = state.completions;
    let bestMatch = Completions.getBestCompletion(completions);
    let maybeMeetPosition =
      completions
      |> Completions.getMeet
      |> Option.map(CompletionMeet.getLocation);
    switch (bestMatch, maybeMeetPosition) {
    | (Some(completion), Some(meetPosition)) =>
      let meet = Location.(meetPosition.column);
      let cursorLocation = Vim.Cursor.getLocation();
      let delta =
        Index.(toZeroBased(cursorLocation.column - toOneBased(meet)));
      Log.infof(m =>
        m(
          "Completing at cursor position: %s | meet: %s",
          Index.show(cursorLocation.column),
          Index.show(meet),
        )
      );

      let idx = ref(delta);
      while (idx^ >= 0) {
        let _ = Vim.input("<BS>");
        decr(idx);
      };

      let latestCursors = ref([]);
      Zed_utf8.iter(
        s => {
          latestCursors := Vim.input(Zed_utf8.singleton(s));
          ();
        },
        completion.item.label,
      );

      state
      |> Selectors.getActiveEditorGroup
      |> Selectors.getActiveEditor
      |> Option.map(Editor.getId)
      |> Option.iter(id => {dispatch(EditorCursorMove(id, latestCursors^))});
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
    | Command("insertBestCompletion") => (state, applyCompletion(state))
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
