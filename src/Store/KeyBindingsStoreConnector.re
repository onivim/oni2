/*
 * KeyBindingsStoreConnector.re
 *
 * This implements an updater (reducer + side effects) for managing key bindings
 */

open Oni_Core;
open Oni_Input;
open Oni_Model;

module Log = (val Log.withNamespace("Oni2.Store.Keybindings"));

module Commands = GlobalCommands;

let start = maybeKeyBindingsFilePath => {
  let default =
    Keybindings.[
      {
        key: "<UP>",
        command: Commands.List.focusUp.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<DOWN>",
        command: Commands.List.focusDown.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<RIGHT>",
        command: Commands.List.selectBackground.id,
        condition: "quickmenuCursorEnd" |> WhenExpr.parse,
      },
      {
        key: "<S-C-F>",
        command: Commands.Workbench.Action.findInFiles.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-F>",
        command: Commands.Workbench.Action.findInFiles.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-TAB>",
        command:
          Commands.Workbench.Action.openNextRecentlyUsedEditorInGroup.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: Commands.Workbench.Action.quickOpen.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: Commands.Workbench.Action.quickOpen.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-C-P>",
        command: Commands.Workbench.Action.showCommands.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-P>",
        command: Commands.Workbench.Action.showCommands.id,
        condition: "editorTextFocus || terminalFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-V>",
        command: Commands.Editor.Action.clipboardPasteAction.id,
        condition: "insertMode || commandLineFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-V>",
        command: Commands.Editor.Action.clipboardPasteAction.id,
        condition: "insertMode || commandLineFocus" |> WhenExpr.parse,
      },
      {
        key: "<ESC>",
        command: Commands.Workbench.Action.closeQuickOpen.id,
        condition: "inQuickOpen" |> WhenExpr.parse,
      },
      {
        key: "<C-N>",
        command: Commands.List.focusDown.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: Commands.List.focusUp.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-N>",
        command: Commands.List.focusDown.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-P>",
        command: Commands.List.focusUp.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: Commands.List.focusDown.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: Commands.List.focusUp.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-TAB>",
        command:
          Commands.Workbench.Action.quickOpenNavigateNextInEditorPicker.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<S-C-TAB>",
        command:
          Commands.Workbench.Action.quickOpenNavigatePreviousInEditorPicker.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<release>",
        command: Commands.List.select.id,
        condition: "inEditorsPicker" |> WhenExpr.parse,
      },
      {
        key: "<CR>",
        command: Commands.List.select.id,
        condition: "listFocus || textInputFocus" |> WhenExpr.parse,
      },
      {
        key: "<S-C-B>",
        command: Commands.Oni.Explorer.toggle.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-P>",
        command: Commands.selectPrevSuggestion.id,
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<C-N>",
        command: Commands.selectNextSuggestion.id,
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<CR>",
        command: Commands.acceptSelectedSuggestion.id,
        condition:
          "acceptSuggestionOnEnter && suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: Commands.acceptSelectedSuggestion.id,
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: Commands.acceptSelectedSuggestion.id,
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<S-CR>",
        command: Commands.acceptSelectedSuggestion.id,
        condition: "suggestWidgetVisible" |> WhenExpr.parse,
      },
      {
        key: "<D-Z>",
        command: Commands.undo.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S-Z>",
        command: Commands.redo.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<D-S>",
        command: Commands.Workbench.Action.Files.save.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-S>",
        command: Commands.Workbench.Action.Files.save.id,
        condition: "editorTextFocus" |> WhenExpr.parse,
      },
      {
        key: "<C-]>",
        command: Commands.Editor.Action.indentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<C-[>",
        command: Commands.Editor.Action.outdentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<D-]>",
        command: Commands.Editor.Action.indentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<D-[>",
        command: Commands.Editor.Action.outdentLines.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<TAB>",
        command: Commands.indent.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<S-TAB>",
        command: Commands.outdent.id,
        condition: "visualMode" |> WhenExpr.parse,
      },
      {
        key: "<C-G>",
        command: Feature_Sneak.Commands.start.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<ESC>",
        command: Feature_Sneak.Commands.stop.id,
        condition: "sneakMode" |> WhenExpr.parse,
      },
      {
        key: "<S-C-M>",
        command: Commands.Workbench.Actions.View.problems.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-M>",
        command: Commands.Workbench.Actions.View.problems.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-W>",
        command: Commands.View.closeEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<C-PAGEDOWN>",
        command: Commands.Workbench.Action.nextEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-]>",
        command: Commands.Workbench.Action.nextEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<C-PAGEUP>",
        command: Commands.Workbench.Action.previousEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-S-[>",
        command: Commands.Workbench.Action.previousEditor.id,
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-=>",
        command: "workbench.action.zoomIn",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<C-=>",
        command: "workbench.action.zoomIn",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-->",
        command: "workbench.action.zoomOut",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<C-->",
        command: "workbench.action.zoomOut",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<D-0>",
        command: "workbench.action.zoomReset",
        condition: WhenExpr.Value(True),
      },
      {
        key: "<C-0>",
        command: "workbench.action.zoomReset",
        condition: WhenExpr.Value(True),
      },
      // TERMINAL
      // Binding to open normal mode
      {
        key: "<C-\\><C-N>",
        command: Feature_Terminal.Commands.Oni.normalMode.id,
        condition: "terminalFocus && insertMode" |> WhenExpr.parse,
      },
      // Bindings to go from normal / visual mode -> insert mode
      {
        key: "o",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      {
        key: "<S-O>",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      {
        key: "Shift+a",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      {
        key: "a",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      {
        key: "i",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      {
        key: "Shift+i",
        command: Feature_Terminal.Commands.Oni.insertMode.id,
        condition:
          "terminalFocus && normalMode || visualMode" |> WhenExpr.parse,
      },
      //LAYOUT
      {
        key: "<C-W>H",
        command: Feature_Layout.Commands.moveLeft.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-H>",
        command: Feature_Layout.Commands.moveLeft.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>L",
        command: Feature_Layout.Commands.moveRight.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-L>",
        command: Feature_Layout.Commands.moveRight.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>K",
        command: Feature_Layout.Commands.moveUp.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-K>",
        command: Feature_Layout.Commands.moveUp.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>J",
        command: Feature_Layout.Commands.moveDown.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-J>",
        command: Feature_Layout.Commands.moveDown.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>R",
        command: Feature_Layout.Commands.rotateForward.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-R>",
        command: Feature_Layout.Commands.rotateForward.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><C-S-R>", // TODO: Does not work, blocked by bug in editor-input
        command: Feature_Layout.Commands.rotateBackward.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>-",
        command: Feature_Layout.Commands.decreaseVerticalSize.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      //      TODO: Does not work, blocked by bug in editor-input
      //      {
      //        key: "<C-W>+",
      //        command: Feature_Layout.Commands.increaseVerticalSize.id,
      //        condition: "!insertMode" |> WhenExpr.parse
      //      },
      {
        key: "<C-W><S-,>", // TODO: Does not work and should be `<`, but blocked by bugs in editor-input,
        command: Feature_Layout.Commands.increaseHorizontalSize.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W><S-.>", // TODO: Does not work and should be `>`, but blocked by bugs in editor-input
        command: Feature_Layout.Commands.decreaseHorizontalSize.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
      {
        key: "<C-W>=",
        command: Feature_Layout.Commands.resetSizes.id,
        condition: "!insertMode" |> WhenExpr.parse,
      },
    ];

  let getKeybindingsFile = () => {
    Filesystem.getOrCreateConfigFile(
      ~overridePath=?maybeKeyBindingsFilePath,
      "keybindings.json",
    );
  };

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
      let keyBindingsFile = getKeybindingsFile();

      let checkFirstLoad = keyBindingPath =>
        if (isFirstLoad) {
          reloadConfigOnWritePost(~configPath=keyBindingPath, dispatch);
        };

      let onError = msg => {
        let errorMsg = "Error parsing keybindings: " ++ msg;
        Log.error(errorMsg);
        dispatch(Actions.KeyBindingsParseError(errorMsg));
      };

      let (keyBindings, individualErrors) =
        keyBindingsFile
        |> Utility.ResultEx.tap(checkFirstLoad)
        |> Utility.ResultEx.flatMap(Utility.JsonEx.from_file)
        |> Utility.ResultEx.flatMap(
             Keybindings.of_yojson_with_errors(~default),
           )
        // Handle error case when parsing entire JSON file
        |> Utility.ResultEx.tapError(onError)
        |> Stdlib.Result.value(~default=(Keybindings.empty, []));

      // Handle individual binding errors
      individualErrors |> List.iter(onError);

      Log.infof(m =>
        m("Loading %i keybindings", Keybindings.count(keyBindings))
      );

      dispatch(Actions.KeyBindingsSet(keyBindings));
    });

  let executeCommandEffect = msg =>
    Isolinear.Effect.createWithDispatch(
      ~name="keybindings.executeCommand", dispatch =>
      switch (msg) {
      | `Arg0(msg) => dispatch(msg)
      | `Arg1(msgf) => dispatch(msgf(Json.Encode.null))
      }
    );

  let updater = (state: State.t, action: Actions.t) => {
    switch (action) {
    | Actions.Init => (state, loadKeyBindingsEffect(true))

    | Actions.KeyBindingsReload => (state, loadKeyBindingsEffect(false))

    | Actions.KeyBindingsParseError(msg) => (
        state,
        Feature_Notification.Effects.create(~kind=Error, msg)
        |> Isolinear.Effect.map(msg => Actions.Notification(msg)),
      )

    | KeybindingInvoked({command}) =>
      switch (Command.Lookup.get(command, State.commands(state))) {
      | Some((command: Command.t(_))) => (
          state,
          executeCommandEffect(command.msg),
        )
      | None =>
        Log.errorf(m => m("Unknown command: %s", command));
        (state, Isolinear.Effect.none);
      }

    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
