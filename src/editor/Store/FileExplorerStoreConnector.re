/**
   FileExplorerStoreConnector.re

   Implements an updater (reducer + side effects) for the File Explorer
 */
open Oni_Core;
open Oni_Model;

let start = () => {
  let (stream, dispatch) = Isolinear.Stream.create();

  let getExplorerFilesEffect = (cwd, languageInfo, iconTheme, ignored) => {
    Isolinear.Effect.createWithDispatch(~name="explorer.open", dispatch => {
      let newTree =
        FileExplorer.getDirectoryTree(cwd, languageInfo, iconTheme, ignored);
      dispatch(Actions.SetExplorerTree(newTree));
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
          ["node_modules", "_esy"],
        ),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };
  (updater, stream);
};
