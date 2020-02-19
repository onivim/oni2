/*
 * KeyBindingsStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing key bindings
 */

open Oni_Core;
open Oni_Input;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Keybindings"));

let start = () => {
  // Helper function for parsing default expressions
  let parseExp = stringExpression =>
    switch (WhenExpression.parse(stringExpression)) {
    | Ok(v) => v
    | Error(msg) => failwith(msg)
    };

  let defaultBindings =
    Keybindings.Keybinding.[
      {
        key: "<UP>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<DOWN>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<RIGHT>",
        command: "list.selectBackground",
        condition: "quickmenuCursorEnd" |> parseExp,
      },
      {
        key: "<S-C-F>",
        command: "workbench.action.findInFiles",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<D-S-F>",
        command: "workbench.action.findInFiles",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<C-TAB>",
        command: "workbench.action.openNextRecentlyUsedEditorInGroup",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<C-P>",
        command: "workbench.action.quickOpen",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<D-P>",
        command: "workbench.action.quickOpen",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<S-C-P>",
        command: "workbench.action.showCommands",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<D-S-P>",
        command: "workbench.action.showCommands",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<C-V>",
        command: "editor.action.clipboardPasteAction",
        condition: "insertMode" |> parseExp,
      },
      {
        key: "<D-V>",
        command: "editor.action.clipboardPasteAction",
        condition: "insertMode" |> parseExp,
      },
      {
        key: "<ESC>",
        command: "workbench.action.closeQuickOpen",
        condition: "inQuickOpen" |> parseExp,
      },
      {
        key: "<C-N>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<C-P>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<D-N>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<D-P>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<TAB>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<S-TAB>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<C-TAB>",
        command: "workbench.action.quickOpenNavigateNextInEditorPicker",
        condition: "inEditorsPicker" |> parseExp,
      },
      {
        key: "<S-C-TAB>",
        command: "workbench.action.quickOpenNavigatePreviousInEditorPicker",
        condition: "inEditorsPicker" |> parseExp,
      },
      {
        key: "<CR>",
        command: "list.select",
        condition: "listFocus || textInputFocus" |> parseExp,
      },
      {
        key: "<S-C-B>",
        command: "explorer.toggle",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<C-P>",
        command: "selectPrevSuggestion",
        condition: "suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<C-N>",
        command: "selectNextSuggestion",
        condition: "suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<CR>",
        command: "acceptSelectedSuggestion",
        condition:
          "acceptSuggestionOnEnter && suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<TAB>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<S-TAB>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<S-CR>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> parseExp,
      },
      {
        key: "<D-Z>",
        command: "undo",
        condition: "editorTextFocus" |> parseExp,
      },
      {
        key: "<D-S-Z>",
        command: "redo",
        condition: "editorTextFocus" |> parseExp,
      },
      {key: "<C-G>", command: "sneak.start", condition: WhenExpression.True},
      {
        key: "<ESC>",
        command: "sneak.stop",
        condition: "sneakMode" |> parseExp,
      },
      {
        key: "<S-C-M>",
        command: "workbench.actions.view.problems",
        condition: WhenExpression.True,
      },
      {
        key: "<D-S-M>",
        command: "workbench.actions.view.problems",
        condition: WhenExpression.True,
      },
      {
        key: "<D-W>",
        command: "view.closeEditor",
        condition: WhenExpression.True,
      },
    ];

  let reloadConfigOnWritePost = (~configPath, dispatch) => {
    let _: unit => unit =
      Vim.AutoCommands.onDispatch((cmd, buffer) => {
        let bufferFileName =
          switch (Vim.Buffer.getFilename(buffer)) {
          | None => ""
          | Some(fileName) => fileName
          };
        if (bufferFileName == configPath && cmd == Vim.Types.BufWritePost) {
          Log.info("Reloading key bindings from: " ++ configPath);
          dispatch(Actions.KeyBindingsReload);
        };
      });
    ();
  };

  let loadKeyBindingsEffect = isFirstLoad =>
    Isolinear.Effect.createWithDispatch(~name="keyBindings.load", dispatch => {
      let keyBindingsFile =
        Filesystem.getOrCreateConfigFile("keybindings.json");

      let keyBindings =
        switch (keyBindingsFile) {
        | Error(msg) =>
          Log.error("Unable to load keybindings: " ++ msg);
          Keybindings.empty;
        | Ok(keyBindingPath) =>
          if (isFirstLoad) {
            reloadConfigOnWritePost(~configPath=keyBindingPath, dispatch);
          };

          let parseResult =
            Yojson.Safe.from_file(keyBindingPath)
            |> Keybindings.of_yojson_with_errors;

          switch (parseResult) {
          | Ok((bindings, _)) => bindings
          | Error(msg) =>
            Log.error("Error parsing keybindings: " ++ msg);
            Keybindings.empty;
          };
        };

      Log.infof(m => m("Loading %i keybindings", List.length(keyBindings)));

      dispatch(Actions.KeyBindingsSet(defaultBindings @ keyBindings));
    });

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init(_) => (state, loadKeyBindingsEffect(true))
    | Actions.KeyBindingsReload => (state, loadKeyBindingsEffect(false))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
