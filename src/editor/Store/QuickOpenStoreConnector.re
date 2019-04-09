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

  let stringToCommand = (parentDir: string, fullPath: string) =>
    Model.Actions.{
      category: None,
      name: getDisplayPath(fullPath, parentDir),
      command: () => Model.Actions.OpenFileByPath(fullPath),
      icon: Some({||}),
    };

  let createQuickOpen = setItems => {
    /* TODO: Track 'currentDirectory' in state as part of a workspace type  */
    let currentDirectory = Rench.Environment.getWorkingDirectory();

    let dispose =
      rg.search(
        currentDirectory,
        items => {
          let result =
            items
            |> List.filter(item => !Sys.is_directory(item))
            |> List.map(stringToCommand(currentDirectory));

          setItems(result);
        },
      );

    dispose;
  };

  let openQuickOpenEffect =
    Isolinear.Effect.createWithDispatch(~name="quickOpen.show", dispatch => {
      dispatch(Model.Actions.MenuOpen(createQuickOpen));
      dispatch(Model.Actions.SetInputControlMode(TextInputFocus));
    });

  let updater = (state, action) =>
    switch (action) {
    | Model.Actions.QuickOpen => (state, openQuickOpenEffect)
    | _ => (state, Isolinear.Effect.none)
    };

  updater;
};
