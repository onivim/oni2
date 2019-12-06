/**
   FileExplorerStoreConnector.re

   Implements an updater (reducer + side effects) for the File Explorer
 */
open Oni_Core;
open Oni_Model;

module Option = Utility.Option;

module Effects = {
  let load = (directory, languageInfo, iconTheme, configuration, ~onComplete) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.load", dispatch => {
      let ignored =
        Configuration.getValue(c => c.filesExclude, configuration);
      let tree =
        FileExplorer.getDirectoryTree(
          directory,
          languageInfo,
          iconTheme,
          ignored,
        );

      dispatch(onComplete(tree));
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

  let openFoldersInPath = (state: State.t, maybePath) => {
    let noop = (state.fileExplorer.tree, Isolinear.Effect.none);

    let (tree, eff) =
      switch (maybePath, state.fileExplorer.tree, state.workspace) {
      | (Some(path), Some(tree), Some({workingDirectory, _})) =>
        let re = Str.regexp_string(workingDirectory ++ Filename.dir_sep);
        let localPath = path |> Str.replace_first(re, "");

        switch (FsTreeNode.findNodesByLocalPath(localPath, tree)) {
        // Nothing to do
        | `Success([])
        | `Failed => noop

        // Load next unloaded node in path
        | `Partial(lastNode) => (
            state.fileExplorer.tree,
            Effects.load(
              lastNode.path,
              state.languageInfo,
              state.iconTheme,
              state.configuration,
              ~onComplete=node =>
              Actions.FileExplorer(FocusNodeLoaded(lastNode.id, node))
            ),
          )

        // Open ALL the nodes (in the path)!
        | `Success(nodes) => (
            Some(
              FsTreeNode.updateNodesInPath(
                ~tree,
                nodes,
                ~updater=FsTreeNode.setOpen,
              ),
            ),
            Isolinear.Effect.none,
          )
        };

      | _ => noop
      };

    ({
       ...state,
       fileExplorer: {
         ...state.fileExplorer,
         tree,
       },
     }, eff);
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
    | TreeLoaded(tree) => (setTree(tree), Isolinear.Effect.none)

    | NodeLoaded(id, node) => (replaceNode(id, node), Isolinear.Effect.none)

    | FocusNodeLoaded(id, node) =>
      openFoldersInPath(replaceNode(id, node), state.fileExplorer.focus);

    | NodeClicked(node) =>
      switch (node) {
      | {kind: File, path, _} => (state, openFileByPathEffect(path))

      | {kind: Directory({isOpen, _}), _} => (
          replaceNode(node.id, FsTreeNode.toggleOpen(node)),
          isOpen
            ? Isolinear.Effect.none
            : Effects.load(
                node.path,
                state.languageInfo,
                state.iconTheme,
                state.configuration,
                ~onComplete=newNode =>
                Actions.FileExplorer(NodeLoaded(node.id, newNode))
              ),
        )
      }
    };
  };

  (
    (state: State.t, action: Actions.t) =>
      switch (action) {
      // TODO: Should be handle by a more general init mechanism
      | Init =>
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
              ~onComplete=tree =>
              Actions.FileExplorer(TreeLoaded(tree))
            ),
            TitleStoreConnector.Effects.updateTitle(newState),
          ]),
        );

      // TODO: These should not be handled here.
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
      | BufferEnter({filePath, _}, _) =>
        let (state, eff) = openFoldersInPath(state, filePath);

        (
          {
            ...state,
            fileExplorer: {
              ...state.fileExplorer,
              focus: filePath,
            },
          },
          eff,
        );

      | FileExplorer(action) => updater(state, action)
      | _ => (state, Isolinear.Effect.none)
      },
    stream,
  );
};
