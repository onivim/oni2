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

  let windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | EditorGroupSelected(_) => FocusManager.push(Editor, s)
    | EditorTabClicked(editorId) => {
        ...s,
        editorGroups: EditorGroups.setActiveEditor(~editorId, s.editorGroups),
      }
    | ViewCloseEditor(editorId) =>
      /* When an editor is closed... lets see if any window splits are empty */

      let editorGroups =
        Model.EditorGroups.closeEditor(~editorId, s.editorGroups);

      /* Remove splits */
      let layout =
        s.layout
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
             s.layout,
           );

      {...s, editorGroups, layout};

    | OpenFileByPath(_) => FocusManager.push(Editor, s)

    | Command("view.rotateForward") =>
      switch (EditorGroups.getActiveEditorGroup(s.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...s,
          layout:
            Feature_Layout.rotateForward(editorGroup.editorGroupId, s.layout),
        }
      | None => s
      }

    | Command("view.rotateBackward") =>
      switch (EditorGroups.getActiveEditorGroup(s.editorGroups)) {
      | Some((editorGroup: EditorGroup.t)) => {
          ...s,
          layout:
            Feature_Layout.rotateBackward(
              editorGroup.editorGroupId,
              s.layout,
            ),
        }
      | None => s
      }

    | Command("workbench.action.decreaseViewSize") =>
      s |> resize(`Horizontal, 0.95) |> resize(`Vertical, 0.95)

    | Command("workbench.action.increaseViewSize") =>
      s |> resize(`Horizontal, 1.05) |> resize(`Vertical, 1.05)

    | Command("vim.decreaseHorizontalWindowSize") =>
      s |> resize(`Horizontal, 0.95)

    | Command("vim.increaseHorizontalWindowSize") =>
      s |> resize(`Horizontal, 1.05)

    | Command("vim.decreaseVerticalWindowSize") =>
      s |> resize(`Vertical, 0.95)

    | Command("vim.increaseVerticalWindowSize") =>
      s |> resize(`Vertical, 1.05)

    | Command("workbench.action.evenEditorWidths") => {
        ...s,
        layout: Feature_Layout.resetWeights(s.layout),
      }

    | WindowHandleDragged({path, delta}) => {
        ...s,
        layout: Feature_Layout.resizeSplit(~path, ~delta, s.layout),
      }

    | _ => s
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
