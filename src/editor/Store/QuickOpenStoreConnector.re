/*
 * QuickOpenStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let start = (rg: Core.Ripgrep.t) => {
  let getDisplayPath = (fullPath, dir) => {
    let re = Str.regexp_string(dir ++ Filename.dir_sep);
    Str.replace_first(re, "", fullPath);
  };

  let stringToCommand:
    (Model.LanguageInfo.t, Model.IconTheme.t, string, string) =>
    Model.Actions.menuCommand =
    (languageInfo, iconTheme, parentDir, fullPath) => {
      category: None,
      name: getDisplayPath(fullPath, parentDir),
      command: () => Model.Actions.OpenFileByPath(fullPath, None),
      icon: Model.FileExplorer.getFileIcon(languageInfo, iconTheme, fullPath),
    };

  let createQuickOpen = (languageInfo, iconTheme, setItems) => {
    /* TODO: Track 'currentDirectory' in state as part of a workspace type  */
    let currentDirectory = Rench.Environment.getWorkingDirectory();

    let dispose =
      rg.search(
        currentDirectory,
        items => {
          let result =
            items
            |> List.filter(item => !Sys.is_directory(item))
            |> List.map(
                 stringToCommand(languageInfo, iconTheme, currentDirectory),
               );

          setItems(result);
        },
      );

    dispose;
  };

  let openQuickOpenEffect = (languageInfo, iconTheme) =>
    Isolinear.Effect.createWithDispatch(~name="quickOpen.show", dispatch => {
      dispatch(
        Model.Actions.MenuOpen(createQuickOpen(languageInfo, iconTheme)),
      );
      dispatch(Model.Actions.SetInputControlMode(TextInputFocus));
    });

  let updater = (state: Model.State.t, action) => {
    switch (action) {
    | Model.Actions.QuickOpen => (
        state,
        openQuickOpenEffect(state.languageInfo, state.iconTheme),
      )
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
