/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let quitEffect =
    Isolinear.Effect.create(~name="windows.quitEffect", () =>
      dispatch(Model.Actions.Quit(false))
    );

  let initializeDefaultViewEffect = (state: State.t) =>
    Isolinear.Effect.create(~name="windows.init", () => {
      let editor =
        WindowTree.createSplit(
          ~editorGroupId=EditorGroups.activeGroupId(state.editorGroups),
          (),
        );

      dispatch(AddSplit(Vertical, editor));
    });

  let windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | WindowSetActive(splitId, _) => {
        ...s,
        windowManager: {
          ...s.windowManager,
          activeWindowId: splitId,
        },
      }
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

  // This effect 'steals' focus from Revery. When we're selecting an active editor,
  // we want to be sure the editor has focus, not a Revery UI component.
  let getFocusEffect =
    Isolinear.Effect.create(~name="windows.getFocus", () => {
      Revery.UI.Focus.loseFocus()
    });

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | Model.Actions.Tick(_) => (state, Isolinear.Effect.none)
    | action =>
      let state = windowUpdater(state, action);

      let effect =
        switch (action) {
        | Init => initializeDefaultViewEffect(state)
        // When opening a file, ensure that the active editor is getting focus
        | OpenFileByPath(_) => getFocusEffect
        | ViewCloseEditor(_) =>
          if (List.length(
                WindowTree.getSplits(state.windowManager.windowTree),
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

  (updater, stream);
};
