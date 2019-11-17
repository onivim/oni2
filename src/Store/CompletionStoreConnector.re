/*
 * CompletionStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for completion
 */

open Oni_Core;
open Oni_Core.Types;
module Model = Oni_Model;

module Actions = Model.Actions;
module Animation = Model.Animation;
module Quickmenu = Model.Quickmenu;

open Actions;

type lastCompletionMeet = Actions.completionMeet;

let defaultMeet = {
  completionMeetBufferId: (-1),
  completionMeetLine: (-1),
  completionMeetColumn: (-1),
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

      switch (editor) {
      | None => ()
      | Some(ed) =>
        let bufferId = ed.bufferId;

        let bufferOpt = Model.Buffers.getBuffer(bufferId, state.buffers);
        switch (bufferOpt) {
        | None => ()
        | Some(buffer) =>
          let line = Index.toInt0(ed.cursorPosition.line);
          let meetOpt =
            Model.CompletionMeet.getMeetFromBufferCursor(
              ~cursor=ed.cursorPosition,
              buffer,
            );
          switch (meetOpt) {
          | None =>
            lastMeet := defaultMeet;
            dispatch(Actions.CompletionEnd);
          | Some(meet) =>
            open Model.CompletionMeet;
            let column = meet.index;
            let _base = meet.base;
            // Check if our 'meet' position has changed
            let newMeet = {
              completionMeetBufferId: bufferId,
              completionMeetLine: line,
              completionMeetColumn: column,
            };
            if (!equals(~line, ~column, ~bufferId, lastMeet^)) {
              Log.info(
                "[Completion] New completion meet: "
                ++ Model.CompletionMeet.show(meetOpt),
              );
              dispatch(Actions.CompletionStart(newMeet));
            } else if
              // If we're at the same position... but our base is different...
              // fire a base change
              (!String.equal(meet.base, lastBase^)) {
              lastBase := meet.base;
              dispatch(Actions.CompletionBaseChanged(meet.base));
              Log.info("[Completion] New completion base: " ++ meet.base);
            };
            lastMeet := newMeet;
          };
        };
      };
    });

  let updater = (state: Model.State.t, action: Actions.t) => {
    switch (action) {
    | Actions.ChangeMode(mode) when mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | Actions.BufferUpdate(_) when state.mode == Vim.Types.Insert => (
        state,
        checkCompletionMeet(state),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
