/*
 * WindowStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for window management
 */

module Core = Oni_Core;
module Model = Oni_Model;

open Model;

let start = getState => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let quitEffect =
    Isolinear.Effect.create(~name="windows.quitEffect", () =>
      dispatch(Model.Actions.Quit(false))
    );

  /**
     We wrap each split component as we have to have a type signature
     that matches unit => React.syntheticElement this is because
     in the WindowManager module we cannot pass a reference of state
     in the type signature e.g. State.t => React.syntheticElement because
     this would cause a circular reference.

     Alternatives are type parameters but this invloves a lot of unrelated
     type params being added everywhere. ?Functors is another route
   */
  let splitFactory = (fn, ()) => {
    let state = getState();
    fn(state);
  };

  let initializeDefaultViewEffect = (state: State.t) =>
    Isolinear.Effect.create(~name="windows.init", () => {
      open WindowManager;
      open WindowTree;
      open Oni_UI;

      let dock =
        registerDock(
          ~order=1,
          ~width=50,
          ~id=MainDock,
          ~component=splitFactory(state => <Dock state />),
          (),
        );

      let editorGroupId = state.editorGroups.activeId;

      let editor = createSplit(~editorGroupId, ());

      let explorer =
        registerDock(
          ~order=2,
          ~width=225,
          ~id=ExplorerDock,
          ~component=splitFactory(state => <FileExplorerView state />),
          (),
        );

      dispatch(RegisterDockItem(dock));
      dispatch(RegisterDockItem(explorer));
      dispatch(AddSplit(Vertical, editor));
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
    | _ => s
    };

  let updater = (state: Model.State.t, action: Model.Actions.t) =>
    if (action === Model.Actions.Tick) {
      (state, Isolinear.Effect.none);
    } else {
      let state = windowUpdater(state, action);

      let effect =
        switch (action) {
        | Init => initializeDefaultViewEffect(state)
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
