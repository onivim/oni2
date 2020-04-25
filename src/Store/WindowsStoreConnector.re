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

  let initializeDefaultViewEffect = (state: State.t) =>
    Isolinear.Effect.createWithDispatch(~name="windows.init", dispatch => {
      dispatch(
        Actions.AddSplit(
          `Vertical,
          EditorGroups.activeGroupId(state.editorGroups),
        ),
      )
    });

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
    | AddSplit(direction, split) => {
        ...s,
        // Fix #686: If we're adding a split, we should turn off zen mode... unless it's the first split being added.
        zenMode: s.zenMode && Feature_Layout.windows(s.layout) == [],
        layout:
          Feature_Layout.addWindow(
            ~target={
              EditorGroups.getActiveEditorGroup(s.editorGroups)
              |> Option.map((group: EditorGroup.t) => group.editorGroupId);
            },
            ~position=`After,
            direction,
            split,
            s.layout,
          ),
      }

    | RemoveSplit(id) => {
        ...s,
        zenMode: false,
        layout: Feature_Layout.removeWindow(id, s.layout),
      }

    | ViewCloseEditor(_) =>
      /* When an editor is closed... lets see if any window splits are empty */

      /* Remove splits */
      let layout =
        s.layout
        |> Feature_Layout.windows
        |> List.filter(editorGroupId =>
             Model.EditorGroups.isEmpty(editorGroupId, s.editorGroups)
           )
        |> List.fold_left(
             (acc, editorGroupId) =>
               Feature_Layout.removeWindow(editorGroupId, acc),
             s.layout,
           );

      {...s, layout};

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

    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
    let state = windowUpdater(state, action);

    let effect =
      switch (action) {
      | Init => initializeDefaultViewEffect(state)
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
