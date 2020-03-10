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
  let defaultBindings =
    Keybindings.Keybinding.[
      {
        key: "<UP>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<DOWN>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<RIGHT>",
        command: "list.selectBackground",
        condition: "quickmenuCursorEnd" |> WhenExpr.parse,
      },
      {
        key: "<S-C-F>",
        command: "workbench.action.findInFiles",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-F>",
        command: "workbench.action.findInFiles",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-TAB>",
        command: "workbench.action.openNextRecentlyUsedEditorInGroup",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: "workbench.action.quickOpen",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: "workbench.action.quickOpen",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-C-P>",
        command: "workbench.action.showCommands",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-P>",
        command: "workbench.action.showCommands",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-V>",
        command: "editor.action.clipboardPasteAction",
        condition: "insertMode" |> WhenExpr.parse,
      },
      {
        key: "<D-V>",
        command: "editor.action.clipboardPasteAction",
        condition: "insertMode" |> WhenExpr.parse,
      },
      {
        key: "<ESC>",
        command: "workbench.action.closeQuickOpen",
        condition: "inQuickOpen" |> WhenExpr.parse,
      },
      {
        key: "<C-N>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-N>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: "list.focusDown",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: "list.focusUp",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-TAB>",
        command: "workbench.action.quickOpenNavigateNextInEditorPicker",
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<S-C-TAB>",
        command: "workbench.action.quickOpenNavigatePreviousInEditorPicker",
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<CR>",
        command: "list.select",
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-C-B>",
        command: "explorer.toggle",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: "selectPrevSuggestion",
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<C-N>",
        command: "selectNextSuggestion",
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<CR>",
        command: "acceptSelectedSuggestion",
        condition:
          "acceptSuggestionOnEnter && suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<S-CR>",
        command: "acceptSelectedSuggestion",
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<D-Z>",
        command: "undo",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-Z>",
        command: "redo",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S>",
        command: "workbench.action.files.save",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-S>",
        command: "workbench.action.files.save",
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: "editor.action.indentLines",
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: "editor.action.outdentLines",
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<C-G>",
        command: "sneak.start",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<ESC>",
        command: "sneak.stop",
        condition: "sneakMode" |> WhenExpr.parse,
      },
      {
        key: "<S-C-M>",
        command: "workbench.actions.view.problems",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-M>",
        command: "workbench.actions.view.problems",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-W>",
        command: "view.closeEditor",
        condition: WhenExpr.Value(True),
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
    | Actions.Init => (state, loadKeyBindingsEffect(true))
    | Actions.KeyBindingsReload => (state, loadKeyBindingsEffect(false))
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
