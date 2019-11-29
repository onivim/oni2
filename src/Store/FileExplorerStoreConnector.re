/**
   FileExplorerStoreConnector.re

   Implements an updater (reducer + side effects) for the File Explorer
 */
open Oni_Core;
open Oni_Model;

let start = () => {
  let (stream, _) = Isolinear.Stream.create();

  let getExplorerFilesEffect = (cwd, languageInfo, iconTheme, ignored) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.open", dispatch => {
      let newTree =
        FileExplorer.getDirectoryTree(cwd, languageInfo, iconTheme, ignored);
      dispatch(Actions.SetExplorerTree(newTree));
    });
  };

  let updateFolderEffect =
      (~folder, ~nodeId, ~languageInfo, ~iconTheme, ~ignored, ~tree) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.update", dispatch => {
      let updatedFolder =
        FileExplorer.getDirectoryTree(
          folder,
          languageInfo,
          iconTheme,
          ignored,
        );

      let updater = _ => updatedFolder;

      let tree = UiTree.updateNode(nodeId, tree, ~updater, ());
      dispatch(Actions.SetExplorerTree(tree));
    });
  };

  let setExplorerTreeEffect = tree => {
    Isolinear.Effect.createWithDispatch(~name="explorer.settree", dispatch => {
      dispatch(Actions.SetExplorerTree(tree))
    });
  };

  let openFileByPathEffect = path => {
    Isolinear.Effect.createWithDispatch(
      ~name="explorer.openFileByPath", dispatch => {
      dispatch(Actions.OpenFileByPath(path, None, None))
    });
  };

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | OpenExplorer(directory) => (
        state,
        getExplorerFilesEffect(
          directory,
          state.languageInfo,
          state.iconTheme,
          Configuration.getValue(c => c.filesExclude, state.configuration),
        ),
      )

    | ExplorerNodeClicked(node) =>
      switch (node) {
      | {data: {isDirectory: false, path, _}, _} =>
        /* Only open files not directories */
        (state, openFileByPathEffect(path))

      /*
       * If the depth of the clicked node is at the maximum depth its children
       * will have no content so we call the update action which will populate
       * the child node and attach it to the tree

       * NOTE: that we match specifically on a Node with an empty list of children
       * so this action is not recalled if the Node has already been populated
       */
      | {data: {isDirectory: true, depth, _}, children: [], _}
          when depth == Constants.default.maximumExplorerDepth => (
          state,
          switch (state.fileExplorer.directory) {
          | Some(tree) =>
            updateFolderEffect(
              ~folder=node.data.path,
              ~nodeId=node.id,
              ~languageInfo=state.languageInfo,
              ~iconTheme=state.iconTheme,
              ~ignored=
                Configuration.getValue(
                  c => c.filesExclude,
                  state.configuration,
                ),
              ~tree,
            )
          | None => Isolinear.Effect.none
          },
        )

      | {id, data: {isDirectory: true, _}, _} =>
        switch (state.fileExplorer.directory) {
        | Some(tree) =>
          let tree = UiTree.updateNode(id, tree, ());
          (state, setExplorerTreeEffect(tree));
        | None => (state, Isolinear.Effect.none)
        }
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
