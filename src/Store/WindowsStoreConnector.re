/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;
open Model.Actions;

module OptionEx = Core.Utility.OptionEx;

let start = () => {
  let quitEffect =
    Isolinear.Effect.createWithDispatch(~name="windows.quitEffect", dispatch =>
      dispatch(Model.Actions.Quit(false))
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

  let windowUpdater = (state: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | EditorGroupSelected(_) => FocusManager.push(Editor, state)

    | EditorTabClicked(editorId) => {
        ...state,
        editorGroups:
          EditorGroups.setActiveEditor(~editorId, state.editorGroups),
      }
    | ViewCloseEditor(editorId) =>
      /* When an editor is closed... lets see if any window splits are empty */

      let editorGroups =
        Model.EditorGroups.closeEditor(~editorId, state.editorGroups);

      /* Remove splits */
      let layout =
        state.layout
        |> Feature_Layout.windows
        |> List.fold_left(
             (acc, editorGroupId) =>
               if (Model.EditorGroups.getEditorGroupById(
                     editorGroups,
                     editorGroupId,
                   )
                   == None) {
                 Feature_Layout.removeWindow(editorGroupId, acc);
               } else {
                 acc;
               },
             state.layout,
           );

      {...state, editorGroups, layout};

    | OpenFileByPath(_) => FocusManager.push(Editor, state)

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

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
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
