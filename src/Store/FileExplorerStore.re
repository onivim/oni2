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

// Counts the number of axpanded nodes before the node specified by the given path
let nodeOffsetByPath = (tree, path) => {
  let rec loop = (node: FsTreeNode.t, path) =>
    switch (path) {
    | [] => failwith("Well, this is awkward (ie. unreachable)")
    | [focus, ...focusTail] =>
      if (focus != node) {
        `NotFound(node.expandedSubtreeSize);
      } else {
        switch (node.kind) {
        | Directory({isOpen: false, _})
        | File => `Found(0)

        | Directory({isOpen: true, children}) =>
          let rec loopChildren = (count, children) =>
            switch (children) {
            | [] => `NotFound(count)
            | [child, ...childTail] =>
              switch (loop(child, focusTail)) {
              | `Found(subtreeCount) => `Found(count + subtreeCount)
              | `NotFound(subtreeCount) =>
                loopChildren(count + subtreeCount, childTail)
              }
            };
          loopChildren(1, children);
        };
      }
    };

  switch (loop(tree, path)) {
  | `Found(count) => Some(count)
  | `NotFound(_) => None
  };
};

let updateFileExplorer = (updater, state) =>
  State.{...state, fileExplorer: updater(state.fileExplorer)};
let setTree = (tree, state) =>
  updateFileExplorer(s => {...s, tree: Some(tree)}, state);
let setOpen = (isOpen, state) =>
  updateFileExplorer(s => {...s, isOpen}, state);
let setFocus = (focus, state) =>
  updateFileExplorer(s => {...s, focus}, state);
let setScrollOffset = (scrollOffset, state) =>
  updateFileExplorer(s => {...s, scrollOffset}, state);

let revealPath = (state: State.t, path) => {
  switch (state.fileExplorer.tree, state.workspace) {
  | (Some(tree), Some({workingDirectory, _})) =>
    let localPath = Workspace.toRelativePath(workingDirectory, path);

    switch (FsTreeNode.findNodesByLocalPath(localPath, tree)) {
    // Nothing to do
    | `Success([])
    | `Failed => (state, Isolinear.Effect.none)

    // Load next unloaded node in path
    | `Partial(lastNode) => (
        state,
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
    | `Success(nodes) =>
      let tree =
        FsTreeNode.updateNodesInPath(
          ~tree,
          nodes,
          ~updater=FsTreeNode.setOpen,
        );
      let offset =
        switch (nodeOffsetByPath(tree, nodes)) {
        | Some(offset) => `Middle(float(offset))
        | None => state.fileExplorer.scrollOffset
        };

      (
        state |> setTree(tree) |> setScrollOffset(offset),
        Isolinear.Effect.none,
      );
    };

  | _ => (state, Isolinear.Effect.none)
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
    let replaceNode = (nodeId, node) =>
      switch (state.fileExplorer.tree) {
      | Some(tree) =>
        setTree(FsTreeNode.update(nodeId, ~tree, ~updater=_ => node), state)
      | None => state
      };

    switch (action) {
    | TreeLoaded(tree) => (setTree(tree, state), Isolinear.Effect.none)

    | NodeLoaded(id, node) => (replaceNode(id, node), Isolinear.Effect.none)

    | FocusNodeLoaded(id, node) =>
      switch (state.fileExplorer.focus) {
      | Some(path) => revealPath(replaceNode(id, node), path)
      | None => (state, Isolinear.Effect.none)
      }

    | NodeClicked(node) =>
      // Set focus here to avoid scrolling in BufferEnter
      let state = setFocus(Some(node.path), state);

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
      };

    | ScrollOffsetChanged(offset) => (
        setScrollOffset(offset, state),
        Isolinear.Effect.none,
      )
    };
  };

  (
    (state: State.t, action: Actions.t) =>
      switch (action) {
      // TODO: Should be handled by a more general init mechanism
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
          setOpen(true, state),
          Isolinear.Effect.none,
        )
      | RemoveDockItem(WindowManager.ExplorerDock) => (
          setOpen(false, state),
          Isolinear.Effect.none,
        )
      | BufferEnter({filePath, _}, _) =>
        switch (state.fileExplorer) {
        | {focus, _} when focus != filePath =>
          let state = setFocus(filePath, state);
          switch (filePath) {
          | Some(path) => revealPath(state, path)
          | None => (state, Isolinear.Effect.none)
          };

        | _ => (state, Isolinear.Effect.none)
        }

      | FileExplorer(action) => updater(state, action)
      | _ => (state, Isolinear.Effect.none)
      },
    stream,
  );
};
