open Oni_Core;

open Oni_Model;
open Oni_Model.Actions;

let createDefaultCommands = getState => {
  State.(
    Actions.[
      Command.create(
        ~category=Some("Preferences"),
        ~name="Open configuration file",
        ~action=OpenConfigFile("configuration.json"),
        (),
      ),
      Command.create(
        ~category=Some("Preferences"),
        ~name="Open keybindings file",
        ~action=OpenConfigFile("keybindings.json"),
        (),
      ),
      Command.create(
        ~category=Some("Preferences"),
        ~name="Reload configuration",
        ~action=ConfigurationReload,
        (),
      ),
      Command.create(
        ~category=Some("Preferences"),
        ~name="Theme Picker",
        ~action=QuickmenuShow(ThemesPicker),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Close Editor",
        ~action=Command("view.closeEditor"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Toggle Problems (Errors, Warnings)",
        ~action=Command("workbench.actions.view.problems"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Split Editor Vertically",
        ~action=Command("view.splitVertical"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Split Editor Horizontally",
        ~action=Command("view.splitHorizontal"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Enable Zen Mode",
        ~enabled=() => !getState().zenMode,
        ~action=EnableZenMode,
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Disable Zen Mode",
        ~enabled=() => getState().zenMode,
        ~action=DisableZenMode,
        (),
      ),
      Command.create(
        ~category=Some("Input"),
        ~name="Disable Key Displayer",
        ~enabled=() => KeyDisplayer.getEnabled(getState().keyDisplayer),
        ~action=DisableKeyDisplayer,
        (),
      ),
      Command.create(
        ~category=Some("Input"),
        ~name="Enable Key Displayer",
        ~enabled=() => !KeyDisplayer.getEnabled(getState().keyDisplayer),
        ~action=EnableKeyDisplayer,
        (),
      ),
      Command.create(
        ~category=Some("References"),
        ~name="Find all References",
        ~action=Command("references-view.find"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Rotate Windows (Forwards)",
        ~action=Command("view.rotateForward"),
        (),
      ),
      Command.create(
        ~category=Some("View"),
        ~name="Rotate Windows (Backwards)",
        ~action=Command("view.rotateBackward"),
        (),
      ),
      Command.create(
        ~category=Some("Editor"),
        ~name="Copy Active Filepath to Clipboard",
        ~action=CopyActiveFilepathToClipboard,
        (),
      ),
      Command.create(
        ~category=None,
        ~name="Goto symbol in file...",
        ~action=QuickmenuShow(DocumentSymbols),
        (),
      ),
    ]
  );
};

let start = (getState, contributedCommands) => {
  let singleActionEffect = (action, name) =>
    Isolinear.Effect.createWithDispatch(~name="command." ++ name, dispatch =>
      dispatch(action)
    );

  let closeEditorEffect = (state, _) =>
    Isolinear.Effect.createWithDispatch(~name="closeEditorEffect", dispatch => {
      let editor =
        Selectors.getActiveEditorGroup(state) |> Selectors.getActiveEditor;

      switch (editor) {
      | None => ()
      | Some(v) => dispatch(ViewCloseEditor(v.editorId))
      };
    });

  let splitEditorEffect = (state, direction, _) =>
    Isolinear.Effect.createWithDispatch(~name="splitEditorEffect", dispatch => {
      let buffer = Selectors.getActiveBuffer(state);

      let newEditorGroup =
        switch (buffer) {
        | Some(b) =>
          let ec = EditorGroup.create();
          let (g, editorId) =
            EditorGroup.getOrCreateEditorForBuffer(ec, Buffer.getId(b));
          let g = EditorGroup.setActiveEditor(g, editorId);
          g;
        | None => EditorGroup.create()
        };

      dispatch(EditorGroupAdd(newEditorGroup));

      let split =
        WindowTree.createSplit(
          ~editorGroupId=newEditorGroup.editorGroupId,
          (),
        );

      dispatch(AddSplit(direction, split));
    });

  let windowMoveEffect = (state: State.t, direction, _) => {
    Isolinear.Effect.createWithDispatch(~name="window.move", dispatch => {
      let windowId = WindowManager.move(direction, state.windowManager);
      let maybeEditorGroupId =
        WindowTree.getEditorGroupIdFromSplitId(
          windowId,
          state.windowManager.windowTree,
        );

      switch (maybeEditorGroupId) {
      | Some(editorGroupId) =>
        dispatch(WindowSetActive(windowId, editorGroupId))
      | None => ()
      };
    });
  };

  let commands = [
    ("keyDisplayer.enable", _ => singleActionEffect(EnableKeyDisplayer)),
    ("keyDisplayer.disable", _ => singleActionEffect(DisableKeyDisplayer)),
    (
      "references-view.find",
      _ => singleActionEffect(References(References.Requested)),
    ),
    (
      "workbench.action.showCommands",
      _ => singleActionEffect(QuickmenuShow(CommandPalette)),
    ),
    (
      "workbench.action.gotoSymbol",
      _ => singleActionEffect(QuickmenuShow(DocumentSymbols)),
    ),
    ("workbench.action.findInFiles", _ => singleActionEffect(SearchHotkey)),
    ("workbench.actions.view.problems", _ => singleActionEffect(DiagnosticsHotKey)),
    (
      "workbench.action.openNextRecentlyUsedEditorInGroup",
      _ => singleActionEffect(QuickmenuShow(EditorsPicker)),
    ),
    (
      "workbench.action.quickOpen",
      _ => singleActionEffect(QuickmenuShow(FilesPicker)),
    ),
    (
      "workbench.action.quickOpenNavigateNextInEditorPicker",
      _ => singleActionEffect(ListFocusDown),
    ),
    (
      "workbench.action.quickOpenNavigatePreviousInEditorPicker",
      _ => singleActionEffect(ListFocusUp),
    ),
    /*(
        "developer.massiveMenu",
        (state: Oni_Model.State.t) => {
          multipleActionEffect([
            MenuOpen(
              (setItems, _, _) => {
                let commands =
                  List.init(1000000, i =>
                    {
                      category: None,
                      name: "Item " ++ string_of_int(i),
                      command: () =>
                        Oni_Model.Actions.ShowNotification(
                          Oni_Model.Notification.create(
                            ~title="derp",
                            ~message=string_of_int(i),
                            (),
                          ),
                        ),
                      icon:
                        Oni_Model.FileExplorer.getFileIcon(
                          state.languageInfo,
                          state.iconTheme,
                          "txt",
                        ),
                    }
                  );
                setItems(commands);
                () => ();
              },
            ),
            SetInputControlMode(TextInputFocus),
          ]);
        },
      ),*/
    (
      "workbench.action.closeQuickOpen",
      _ => singleActionEffect(QuickmenuClose),
    ),
    ("list.focusDown", _ => singleActionEffect(ListFocusDown)),
    ("list.focusUp", _ => singleActionEffect(ListFocusUp)),
    ("list.select", _ => singleActionEffect(ListSelect)),
    ("list.selectBackground", _ => singleActionEffect(ListSelectBackground)),
    ("view.closeEditor", state => closeEditorEffect(state)),
    ("view.splitVertical", state => splitEditorEffect(state, Vertical)),
    ("view.splitHorizontal", state => splitEditorEffect(state, Horizontal)),
    (
      "explorer.toggle",
      _ =>
        singleActionEffect(
          Actions.ActivityBar(ActivityBar.FileExplorerClick),
        ),
    ),
    ("window.moveLeft", state => windowMoveEffect(state, Left)),
    ("window.moveRight", state => windowMoveEffect(state, Right)),
    ("window.moveUp", state => windowMoveEffect(state, Up)),
    ("window.moveDown", state => windowMoveEffect(state, Down)),
  ];

  let commandMap =
    List.fold_left(
      (prev, curr) => {
        let (command, handler) = curr;
        StringMap.add(command, handler, prev);
      },
      StringMap.empty,
      commands,
    );

  let setInitialCommands =
    Isolinear.Effect.createWithDispatch(~name="commands.setInitial", dispatch => {
      let commands = createDefaultCommands(getState) @ contributedCommands;
      dispatch(CommandsRegister(commands));
    });

  let updater = (state: State.t, action) => {
    switch (action) {
    | Init => (state, setInitialCommands)
    | Command(cmd) =>
      switch (StringMap.find_opt(cmd, commandMap)) {
      | Some(v) => (state, v(state, cmd))
      | None => (state, Isolinear.Effect.none)
      }
    | _ => (state, Isolinear.Effect.none)
    };
  };

  updater;
};
