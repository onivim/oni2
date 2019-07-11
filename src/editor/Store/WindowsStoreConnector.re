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
    Isolinear.Effect.create(~name="windows.quitEffect", () => {
      dispatch(Model.Actions.Quit(false));
    });

  let windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
    switch (action) {
    | RegisterDockItem(dock) =>
      switch (dock) {
      | {position: Left, _} => {
          ...s,
          windowManager: {
            ...s.windowManager,
            leftDock: [dock, ...s.windowManager.leftDock],
            dockItems: [dock, ...s.windowManager.dockItems],
          },
        }
      | {position: Right, _} => {
          ...s,
          windowManager: {
            ...s.windowManager,
            rightDock: [dock, ...s.windowManager.rightDock],
            dockItems: [dock, ...s.windowManager.dockItems],
          },
        }
      }
    | RemoveDockItem(id) => {
        ...s,
        windowManager: WindowManager.removeDockItem(~id, s.windowManager),
      }
    | AddDockItem(id) =>
      switch (WindowManager.findDockItem(id, s.windowManager)) {
      | Some(dock) => {
          ...s,
          windowManager: {
            ...s.windowManager,
            leftDock: s.windowManager.leftDock @ [dock],
          },
        }
      | None => s
      }
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
        windowManager: {
          ...s.windowManager,
          windowTree: WindowTree.removeSplit(id, s.windowManager.windowTree),
        },
      }
    | ViewCloseEditor(_) => {
      /* When an editor is closed... lets see if any window splits are empty */

      /* Remove splits */
      let initialSplitCount = List.length(WindowTree.getSplits(s.windowManager.windowTree));
      let windowTree = s.windowManager.windowTree
      |> WindowTree.getSplits
      |> List.filter((split: WindowTree.split) => Model.EditorGroups.isEmpty(split.editorGroupId, s.editorGroups))
      |> List.fold_left((prev: WindowTree.t, curr: WindowTree.split) => {
          WindowTree.removeSplit(curr.id, prev)
      }, s.windowManager.windowTree);

      let windowManager = WindowManager.ensureActive({
        ...s.windowManager,
        windowTree,
      });
      
      let newSplitCount = List.length(WindowTree.getSplits(windowManager.windowTree));

      {
        ...s,
        windowManager
      }
    }
    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    if (action === Model.Actions.Tick) {
      (state, Isolinear.Effect.none);
    } else {
      let state = windowUpdater(state, action);

      let effect = switch(action) {
      | ViewCloseEditor(_) => 
        if(List.length(WindowTree.getSplits(state.windowManager.windowTree)) == 0) {
          quitEffect
        } else {
          Isolinear.Effect.none
        }
      | _ => Isolinear.Effect.none
      };

      (state, effect);
    };

  (updater, stream);
};
