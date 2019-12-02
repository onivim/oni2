/**
   FileExplorerStoreConnector.re

   Implements an updater (reducer + side effects) for the File Explorer
 */
open Oni_Core;
open Oni_Model;

module Effects = {
  let load = (directory, languageInfo, iconTheme, configuration) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.load", dispatch => {
      let ignored =
        Configuration.getValue(c => c.filesExclude, configuration);
      let newTree =
        FileExplorer.getDirectoryTree(
          directory,
          languageInfo,
          iconTheme,
          ignored,
        );

      dispatch(Actions.FileExplorer(TreeUpdated(newTree)));
    });
  };

  let refreshNode =
      (node: FsTreeNode.t, languageInfo, iconTheme, configuration) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.update", dispatch => {
      let ignored =
        Configuration.getValue(c => c.filesExclude, configuration);
      let prevId = node.id;
      let node =
        FileExplorer.getDirectoryTree(
          node.path,
          languageInfo,
          iconTheme,
          ignored,
        );

      dispatch(Actions.FileExplorer(NodeUpdated(prevId, node)));
    });
  };
};

let start = () => {
  let (stream, _) = Isolinear.Stream.create();

  // TODO: remove when effect has been exposed wherever that ought to be (VimStore?)
  let openFileByPathEffect = path => {
    Isolinear.Effect.createWithDispatch(
      ~name="explorer.openFileByPath", dispatch => {
      dispatch(Actions.OpenFileByPath(path, None, None))
    });
  };

  let updater = (state: State.t, action: FileExplorer.action) => {
    let setTree = tree => {
      ...state,
      fileExplorer: {
        ...state.fileExplorer,
        tree: Some(tree),
      },
    };

    let replaceNode = (nodeId, node) =>
      switch (state.fileExplorer.tree) {
      | Some(tree) =>
        setTree(FsTreeNode.update(nodeId, ~tree, ~updater=_ => node))
      | None => state
      };

    switch (action) {
    | TreeUpdated(tree) => (setTree(tree), Isolinear.Effect.none)

    | NodeUpdated(id, node) => (
        replaceNode(id, node),
        Isolinear.Effect.none,
      )

    | NodeClicked(node) =>
      switch (node) {
      | {kind: File, path, _} => (state, openFileByPathEffect(path))

      | {kind: Directory({children: `Loading, _}), _} => (
          state,
          Effects.refreshNode(
            node,
            state.languageInfo,
            state.iconTheme,
            state.configuration,
          ),
        )

      | _ => (
          replaceNode(node.id, FsTreeNode.toggleOpenState(node)),
          Isolinear.Effect.none,
        )
      }
    };
  };

  (
    (state: State.t) =>
      fun
      // TODO: Should be handle by a more general init mechanism
      | Actions.Init => {
          let cwd = Rench.Environment.getWorkingDirectory();
          let newState = {
            ...state,
            workspace:
              Some(
                Workspace.{
                  workingDirectory: cwd,
                  rootName: Filename.basename(cwd),
                },
              ),
          };

          (
            newState,
            Isolinear.Effect.batch([
              Effects.load(
                cwd,
                state.languageInfo,
                state.iconTheme,
                state.configuration,
              ),
              TitleStoreConnector.Effects.updateTitle(newState),
            ]),
          );
        }

      // TODO: Should not be handled here.
      | AddDockItem(WindowManager.ExplorerDock) => (
          {
            ...state,
            fileExplorer: {
              ...state.fileExplorer,
              isOpen: true,
            },
          },
          Isolinear.Effect.none,
        )
      | RemoveDockItem(WindowManager.ExplorerDock) => (
          {
            ...state,
            fileExplorer: {
              ...state.fileExplorer,
              isOpen: false,
            },
          },
          Isolinear.Effect.none,
        )

      | Actions.FileExplorer(action) => updater(state, action)
      | _ => (state, Isolinear.Effect.none),
    stream,
  );
};
