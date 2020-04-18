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
      let editor =
        Feature_Layout.WindowTree.createSplit(
          ~editorGroupId=EditorGroups.activeGroupId(state.editorGroups),
          (),
        );

      dispatch(Actions.AddSplit(Vertical, editor));
    });

  let windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | WindowTreeSetSize(width, height) => {
        ...s,
        layout: Feature_Layout.setTreeSize(width, height, s.layout),
      }

    | AddSplit(direction, split) => {
        ...s,
        // Fix #686: If we're adding a split, we should turn off zen mode... unless it's the first split being added.
        zenMode:
          s.zenMode
          && List.length(
               Feature_Layout.WindowTree.getSplits(s.layout.windowTree),
             )
          == 0,
        layout: {
          ...s.layout,
          windowTree:
            Feature_Layout.WindowTree.addSplit(
              ~target={
                EditorGroups.getActiveEditorGroup(s.editorGroups)
                |> Option.map((group: EditorGroup.t) => group.editorGroupId);
              },
              ~position=After,
              direction,
              split,
              s.layout.windowTree,
            ),
        },
      }

    | RemoveSplit(id) => {
        ...s,
        zenMode: false,
        layout: {
          ...s.layout,
          windowTree:
            Feature_Layout.WindowTree.removeSplit(id, s.layout.windowTree),
        },
      }

    | ViewCloseEditor(_) =>
      /* When an editor is closed... lets see if any window splits are empty */

      /* Remove splits */
      let windowTree =
        s.layout.windowTree
        |> Feature_Layout.WindowTree.getSplits
        |> List.filter((split: Feature_Layout.WindowTree.split) =>
             Model.EditorGroups.isEmpty(split.editorGroupId, s.editorGroups)
           )
        |> List.fold_left(
             (
               prev: Feature_Layout.WindowTree.t,
               curr: Feature_Layout.WindowTree.split,
             ) =>
               Feature_Layout.WindowTree.removeSplit(curr.editorGroupId, prev),
             s.layout.windowTree,
           );

      {
        ...s,
        layout: {
          ...s.layout,
          windowTree,
        },
      };

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

    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
    let state = windowUpdater(state, action);

    let effect =
      switch (action) {
      | Init => initializeDefaultViewEffect(state)
      // When opening a file, ensure that the active editor is getting focus
      | ViewCloseEditor(_) =>
        if (List.length(
              Feature_Layout.WindowTree.getSplits(state.layout.windowTree),
            )
            == 0) {
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
