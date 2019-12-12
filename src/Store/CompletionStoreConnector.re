/*
 * CompletionStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for completion
 */

open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Quickmenu = Model.Quickmenu;

open Actions;

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

let start = () => {
  let checkCompletionMeet = (state: Model.State.t) =>
    Isolinear.Effect.createWithDispatch(~name="completion.checkMeet", dispatch => {
      let editor =
        state
        |> Model.Selectors.getActiveEditorGroup
        |> Model.Selectors.getActiveEditor;

      editor
      |> Option.iter(ed => {
           let bufferId = ed.bufferId;
           let bufferOpt = Model.Buffers.getBuffer(bufferId, state.buffers);
           switch (bufferOpt) {
           | None => ()
           | Some(buffer) =>
             let cursorPosition = Model.Editor.getPrimaryCursor(ed);
             let meetOpt =
               Model.CompletionMeet.createFromBufferLocation(
                 ~location=cursorPosition,
                 buffer,
               );
             switch (meetOpt) {
             | None =>
               lastMeet := defaultMeet;
               dispatch(Actions.CompletionEnd);
             | Some(meet) =>
               open Model.CompletionMeet;
               let {line, column}: Location.t = getLocation(meet);
               let base = getBase(meet);
               // Check if our 'meet' position has changed
               let newMeet = {
                 completionMeetBufferId: bufferId,
                 completionMeetLine: line,
                 completionMeetColumn: column,
               };
               if (!equals(~line, ~column, ~bufferId, lastMeet^)) {
                 Log.info(
                   "[Completion] New completion meet: "
                   ++ Model.CompletionMeet.toString(meet),
                 );
                 dispatch(Actions.CompletionStart(meet));
                 dispatch(Actions.CompletionBaseChanged(base));
               } else if
                 // If we're at the same position... but our base is different...
                 // fire a base change
                 (!String.equal(base, lastBase^)) {
                 lastBase := base;
                 dispatch(Actions.CompletionBaseChanged(base));
                 Log.info("[Completion] New completion base: " ++ base);
               };
               lastMeet := newMeet;
             };
           };
         });
    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ChangeMode(mode) when mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | Actions.EditorCursorMove(_) when state.mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
