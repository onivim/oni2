/*
 * QuickOpenStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for the Menu
 */

module Core = Oni_Core;
module Model = Oni_Model;

module Extensions = Oni_Extensions;

let ripgrepQueryFromFilter = s => {
  let a =
    s |> String.to_seq |> Seq.map(c => String.make(1, c)) |> List.of_seq;
  let b = String.concat("*", a);
  "*" ++ b ++ "*";
};

let start = (rg: Core.Ripgrep.t(Model.Actions.menuCommand)) => {
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

  let createQuickOpen =
      (languageInfo, iconTheme, setItems, onQueryChanged, setLoading) => {
    /* TODO: Track 'currentDirectory' in state as part of a workspace type  */
    let currentDirectory = Rench.Environment.getWorkingDirectory();

    setLoading(true);

    /* Create a hashtable to keep track of dups */
    let discoveredPaths: Hashtbl.t(string, bool) = Hashtbl.create(1000);

    let filter = item => {
      switch (Hashtbl.find_opt(discoveredPaths, item)) {
      | Some(_) => false
      | None =>
        Hashtbl.add(discoveredPaths, item, true);
        switch (!Sys.is_directory(item)) {
        | exception _ => false
        | v => v
        };
      };
    };

    let search = arg => {
      setLoading(true);
      rg.search(
        stringToCommand(languageInfo, iconTheme, currentDirectory),
        arg,
        currentDirectory,
        items => setItems(items),
        () => {
          setLoading(false);
          Core.Log.info("[QuickOpenStoreConnector] Ripgrep completed.");
        },
      );
    };

    let dispose1 = ref(search("*"));

    let ret = () => {
      let _ = dispose1^();
      ();
    };

    ret;
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
