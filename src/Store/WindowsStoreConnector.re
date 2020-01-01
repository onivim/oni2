/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;

open Oni_Model;
open Actions;

module OptionEx = Core.Utility.OptionEx;

let start = () => {
  let quitEffect =
    Isolinear.Effect.createWithDispatch(~name="windows.quitEffect", dispatch =>
      dispatch(Actions.Quit(false))
    );

  let resize = (axis, factor, state: State.t) =>
    switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
    | Some((editorGroup: EditorGroup.t)) => {
        ...state,
        layout:
          Feature_Layout.resizeWindow(
            axis,
            editorGroup.editorGroupId,
            factor,
            state.layout,
          ),
      }
    | None => state
    };

  let windowUpdater = (state: State.t, action: Actions.t) =>
    switch (action) {
    | EditorGroupSelected(_) => FocusManager.push(Editor, state)

    | WindowMoveLeft =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          editorGroups:
            EditorGroups.setActive(
              Feature_Layout.moveLeft(
                editorGroup.editorGroupId,
                state.layout,
              ),
              state.editorGroups,
            ),
        }
      | None => state
      }

    | WindowMoveRight =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          editorGroups:
            EditorGroups.setActive(
              Feature_Layout.moveRight(
                editorGroup.editorGroupId,
                state.layout,
              ),
              state.editorGroups,
            ),
        }
      | None => state
      }

    | WindowMoveUp =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          editorGroups:
            EditorGroups.setActive(
              Feature_Layout.moveUp(editorGroup.editorGroupId, state.layout),
              state.editorGroups,
            ),
        }
      | None => state
      }

    | WindowMoveDown =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          editorGroups:
            EditorGroups.setActive(
              Feature_Layout.moveDown(
                editorGroup.editorGroupId,
                state.layout,
              ),
              state.editorGroups,
            ),
        }
      | None => state
      }

    | ViewCloseEditor(_) =>
      /* When an editor is closed... lets see if any window splits are empty */

      /* Remove splits */
      let layout =
        state.layout
        |> Feature_Layout.windows
        |> List.filter(editorGroupId =>
             EditorGroups.isEmpty(editorGroupId, state.editorGroups)
           )
        |> List.fold_left(
             (acc, editorGroupId) =>
               Feature_Layout.removeWindow(editorGroupId, acc),
             state.layout,
           );

      {...state, layout};

    | OpenFileByPath(_) => FocusManager.push(Editor, state)

    | WindowRotateForward =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          layout:
            Feature_Layout.rotateForward(
              editorGroup.editorGroupId,
              state.layout,
            ),
        }
      | None => state
      }

    | WindowRotateBackward =>
      switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...state,
          layout:
            Feature_Layout.rotateBackward(
              editorGroup.editorGroupId,
              state.layout,
            ),
        }
      | None => state
      }

    | Command("workbench.action.decreaseViewSize") =>
      state |> resize(`Horizontal, 0.95) |> resize(`Vertical, 0.95)

    | Command("workbench.action.increaseViewSize") =>
      state |> resize(`Horizontal, 1.05) |> resize(`Vertical, 1.05)

    | Command("vim.decreaseHorizontalWindowSize") =>
      state |> resize(`Horizontal, 0.95)

    | Command("vim.increaseHorizontalWindowSize") =>
      state |> resize(`Horizontal, 1.05)

    | Command("vim.decreaseVerticalWindowSize") =>
      state |> resize(`Vertical, 0.95)

    | Command("vim.increaseVerticalWindowSize") =>
      state |> resize(`Vertical, 1.05)

    | Command("workbench.action.evenEditorWidths") => {
        ...state,
        layout: Feature_Layout.resetWeights(state.layout),
      }

    | _ => state
    };

  let updater = (state: State.t, action: Actions.t) => {
    let state = windowUpdater(state, action);

    let effect =
      switch (action) {
      // When opening a file, ensure that the active editor is getting focus
      | ViewCloseEditor(_) =>
        if (Feature_Layout.windows(state.layout) == []) {
          quitEffect;
        } else {
          Isolinear.Effect.none;
        }
      | _ => Isolinear.Effect.none
      };

    (state, effect);
  };

  updater;
};
