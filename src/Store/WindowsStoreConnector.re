/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;
open Model.Actions;

let start = () => {
  let quitEffect =
    Isolinear.Effect.createWithDispatch(~name="windows.quitEffect", dispatch =>
      dispatch(Model.Actions.Quit(false))
    );

  let initializeDefaultViewEffect = (state: State.t) =>
    Isolinear.Effect.createWithDispatch(~name="windows.init", dispatch => {
      let editor =
        WindowTree.createSplit(
          ~editorGroupId=EditorGroups.activeGroupId(state.editorGroups),
          (),
        );

      dispatch(Actions.AddSplit(Vertical, editor));
    });

  let windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | WindowSetActive(splitId, _) =>
      {
        ...s,
        windowManager: {
          ...s.windowManager,
          activeWindowId: splitId,
        },
      }
      |> FocusManager.push(Editor)

    | WindowTreeSetSize(width, height) => {
        ...s,
        windowManager:
          WindowManager.setTreeSize(width, height, s.windowManager),
      }

    | AddSplit(direction, split) => {
        ...s,
        // Fix #686: If we're adding a split, we should turn off zen mode... unless it's the first split being added.
        zenMode:
          s.zenMode
          && List.length(WindowTree.getSplits(s.windowManager.windowTree))
          == 0,
        windowManager: {
          ...s.windowManager,
          activeWindowId: split.id,
          windowTree:
            WindowTree.addSplit(
              ~target=Some(s.windowManager.activeWindowId),
              direction,
              split,
              s.windowManager.windowTree,
            ),
        },
      }

    | RemoveSplit(id) => {
        ...s,
        zenMode: false,
        windowManager: {
          ...s.windowManager,
          windowTree: WindowTree.removeSplit(id, s.windowManager.windowTree),
        },
      }

    | ViewCloseEditor(_) =>
      /* When an editor is closed... lets see if any window splits are empty */

      /* Remove splits */
      let windowTree =
        s.windowManager.windowTree
        |> WindowTree.getSplits
        |> List.filter((split: WindowTree.split) =>
             Model.EditorGroups.isEmpty(split.editorGroupId, s.editorGroups)
           )
        |> List.fold_left(
             (prev: WindowTree.t, curr: WindowTree.split) =>
               WindowTree.removeSplit(curr.id, prev),
             s.windowManager.windowTree,
           );

      let windowManager =
        WindowManager.ensureActive({...s.windowManager, windowTree});

      {...s, windowManager};

    | OpenFileByPath(_) => FocusManager.push(Editor, s)

    | Command("view.rotateForward") => {
        ...s,
        windowManager: {
          ...s.windowManager,
          windowTree:
            WindowTree.rotateForward(
              s.windowManager.activeWindowId,
              s.windowManager.windowTree,
            ),
        },
      }

    | Command("view.rotateBackward") => {
        ...s,
        windowManager: {
          ...s.windowManager,
          windowTree:
            WindowTree.rotateBackward(
              s.windowManager.activeWindowId,
              s.windowManager.windowTree,
            ),
        },
      }

    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) => {
    let state = windowUpdater(state, action);

    let effect =
      switch (action) {
      | Init(_) => initializeDefaultViewEffect(state)
      // When opening a file, ensure that the active editor is getting focus
      | ViewCloseEditor(_) =>
        if (List.length(WindowTree.getSplits(state.windowManager.windowTree))
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
