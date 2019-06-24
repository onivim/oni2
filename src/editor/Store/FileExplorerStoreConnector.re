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

      let updater = node =>
        UiTree.(
          switch (node) {
          | Node(_, _) => updatedFolder
          | Empty => Empty
          }
        );

      let updated = UiTree.updateNode(nodeId, tree, ~updater, ());
      dispatch(Actions.SetExplorerTree(updated.tree));
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
    | UpdateExplorerNode(clicked, tree) =>
      let path = FileExplorer.getNodePath(clicked);
      let id = FileExplorer.getNodeId(clicked);
      switch (path, id) {
      | (Some(p), Some(id)) => (
          state,
          updateFolderEffect(
            ~folder=p,
            ~nodeId=id,
            ~languageInfo=state.languageInfo,
            ~iconTheme=state.iconTheme,
            ~ignored=Configuration.getValue(c => c.filesExclude, state.configuration),
            ~tree,
          ),
        )
      | (_, _) => (state, Isolinear.Effect.none)
      };
    | _ => (state, Isolinear.Effect.none)
    };
  };
  (updater, stream);
};
