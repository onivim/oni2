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

  let rec windowUpdater = (s: Model.State.t, action: Model.Actions.t) =>
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
    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    if (action === Model.Actions.Tick) {
      (state, Isolinear.Effect.none);
    } else {
      let state = windowUpdater(state, action);
      (state, Isolinear.Effect.none);
    };

  (updater, stream);
};
