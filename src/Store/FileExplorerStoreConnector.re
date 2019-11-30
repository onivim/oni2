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
      let updater = _ =>
        FileExplorer.getDirectoryTree(
          folder,
          languageInfo,
          iconTheme,
          ignored,
        );

      let tree = UiTree.updateNode(nodeId, tree, ~updater);
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
      | {kind: File, path, _} => (state, openFileByPathEffect(path))

      | {kind: Directory({children: `Loading}), _} => (
          state,
          switch (state.fileExplorer.directory) {
          | Some(tree) =>
            updateFolderEffect(
              ~folder=node.path,
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

      | _ =>
        switch (state.fileExplorer.directory) {
        | Some(tree) =>
          let tree = UiTree.toggleOpenState(node.id, tree);
          (state, setExplorerTreeEffect(tree));
        | None => (state, Isolinear.Effect.none)
        }
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  (updater, stream);
};
