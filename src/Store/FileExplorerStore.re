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

let updateFileExplorer = (updater, state) =>
  State.{...state, fileExplorer: updater(state.fileExplorer)};
let setTree = (tree, state) =>
  updateFileExplorer(s => {...s, tree: Some(tree)}, state);
let setOpen = (isOpen, state) =>
  updateFileExplorer(s => {...s, isOpen}, state);
let setActive = (maybePath, state) =>
  updateFileExplorer(s => {...s, active: maybePath}, state);
let setFocus = (maybePath, state) =>
  updateFileExplorer(s => {...s, focus: maybePath}, state);
let setScrollOffset = (scrollOffset, state) =>
  updateFileExplorer(s => {...s, scrollOffset}, state);

let revealPath = (state: State.t, path) => {
  switch (state.fileExplorer.tree) {
  | Some(tree) =>
    switch (FsTreeNode.findNodesByPath(path, tree)) {
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
          Actions.FileExplorer(FocusNodeLoaded(lastNode.path, node))
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
        switch (FsTreeNode.expandedIndex(tree, nodes)) {
        | Some(offset) => `Middle(float(offset))
        | None => state.fileExplorer.scrollOffset
        };

      (
        state |> setTree(tree) |> setScrollOffset(offset),
        Isolinear.Effect.none,
      );
    }

  | None => (state, Isolinear.Effect.none)
  };
};

let replaceNode = (path, node, state: State.t) =>
  switch (state.fileExplorer.tree) {
  | Some(tree) =>
    setTree(FsTreeNode.update(path, ~tree, ~updater=_ => node), state)
  | None => state
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
    switch (action) {
    | TreeLoaded(tree) => (setTree(tree, state), Isolinear.Effect.none)

    | NodeLoaded(path, node) => (
        replaceNode(path, node, state),
        Isolinear.Effect.none,
      )

    | FocusNodeLoaded(path, node) =>
      switch (state.fileExplorer.active) {
      | Some(activePath) =>
        let state = replaceNode(path, node, state);
        revealPath(state, activePath);
      | None => (state, Isolinear.Effect.none)
      }

    | NodeClicked(node) =>
      let state = state |> setFocus(Some(node.path));

      switch (node) {
      | {kind: File, path, _} =>
        // Set active here to avoid scrolling in BufferEnter
        (state |> setActive(Some(node.path)), openFileByPathEffect(path))

      | {kind: Directory({isOpen, _}), _} => (
          replaceNode(node.path, FsTreeNode.toggleOpen(node), state),
          isOpen
            ? Isolinear.Effect.none
            : Effects.load(
                node.path,
                state.languageInfo,
                state.iconTheme,
                state.configuration,
                ~onComplete=newNode =>
                Actions.FileExplorer(NodeLoaded(node.path, newNode))
              ),
        )
      };

    | ScrollOffsetChanged(offset) => (
        setScrollOffset(offset, state),
        Isolinear.Effect.none,
      )

    | FocusPrev =>
      switch (state.fileExplorer.tree, state.fileExplorer.focus) {
      | (Some(tree), Some(path)) =>
        switch (FsTreeNode.prevExpandedNode(path, tree)) {
        | Some(node) => (
            setFocus(Some(node.path), state),
            Isolinear.Effect.none,
          )
        | None => (state, Isolinear.Effect.none)
        }

      | _ => (state, Isolinear.Effect.none)
      }

    | FocusNext =>
      switch (state.fileExplorer.tree, state.fileExplorer.focus) {
      | (Some(tree), Some(path)) =>
        switch (FsTreeNode.nextExpandedNode(path, tree)) {
        | Some(node) => (
            setFocus(Some(node.path), state),
            Isolinear.Effect.none,
          )
        | None => (state, Isolinear.Effect.none)
        }

      | _ => (state, Isolinear.Effect.none)
      }
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

      | BufferEnter({filePath, _}, _) =>
        switch (state.fileExplorer) {
        | {active, _} when active != filePath =>
          let state = setActive(filePath, state);
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
