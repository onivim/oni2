open WhenExpr.ContextKeys;

let menus = (~isFocused) => {
  Schema.(
    fromList(
      // TODO: This should be factored to a feature...
      isFocused
        ? Quickmenu.[
            bool(
              "commandLineFocus",
              fun
              | Some({variant: Wildmenu(_), _}) => true
              | _ => false,
            ),
            bool("listFocus", model => model != None),
            bool("inQuickOpen", model => model != None),
            bool(
              "inEditorsPicker",
              fun
              | Some({variant: EditorsPicker, _}) => true
              | _ => false,
            ),
            bool(
              "textInputFocus",
              fun
              | Some({variant: EditorsPicker, _}) => false
              | Some(_) => true
              | None => false,
            ),
            bool(
              "quickmenuCursorEnd",
              fun
              | Some({inputText, _})
                  when Component_InputText.isCursorAtEnd(inputText) =>
                true
              | _ => false,
            ),
          ]
        : [],
    )
  );
};

let editors = (~isFocused) => {
  Schema.(
    fromList(
      isFocused
        ? State.[
            bool("editorTextFocus", state =>
              switch (ModeManager.current(state)) {
              | TerminalInsert
              | TerminalNormal
              | TerminalVisual(_) => false
              | _ => true
              }
            ),
            bool("terminalFocus", state =>
              switch (ModeManager.current(state)) {
              | TerminalInsert
              | TerminalNormal
              | TerminalVisual(_) => true
              | _ => false
              }
            ),
            bool("commandLineFocus", state =>
              ModeManager.current(state) == CommandLine
            ),
            bool("insertMode", state =>
              switch (ModeManager.current(state)) {
              | TerminalInsert
              | Insert => true
              | _ => false
              }
            ),
            bool("normalMode", state =>
              switch (ModeManager.current(state)) {
              | TerminalNormal
              | Normal => true
              | _ => false
              }
            ),
            bool("visualMode", state =>
              switch (ModeManager.current(state)) {
              | TerminalVisual(_)
              | Visual(_) => true
              | _ => false
              }
            ),
            bool("parameterHintsVisible", state =>
              Feature_SignatureHelp.isShown(state.signatureHelp)
            ),
          ]
        : [],
    )
  );
};

let other = {
  Schema.(
    fromList(
      State.[
        bool("isLinux", _state =>
          Revery.Environment.os == Revery.Environment.Linux
        ),
        bool("isMac", _state =>
          Revery.Environment.os == Revery.Environment.Mac
        ),
        bool("isWin", _state =>
          Revery.Environment.os == Revery.Environment.Windows
        ),
        bool("sneakMode", state => Feature_Sneak.isActive(state.sneak)),
        bool("zenMode", state => state.zenMode),
        bool("keyDisplayerEnabled", state => state.keyDisplayer != None),
      ],
    )
  );
};

let all = (state: State.t) => {
  let focus = FocusManager.current(state);
  let sideBarContext =
    Feature_SideBar.Contributions.contextKeys(
      // TODO: Replace with Focus.SideBar once state has moved
      ~isFocused=
        focus == Focus.Extensions
        || focus == Focus.FileExplorer
        || focus == Focus.SCM
        || focus == Focus.Search,
    )
    |> Schema.fromList
    |> Schema.map(({sideBar, _}: State.t) => sideBar)
    |> fromSchema(state);

  // TODO: These sidebar-specific UI pieces should be encapsulated
  // by Feature_SideBar.contextKeys.
  let scmContextKeys =
    Feature_SCM.Contributions.contextKeys(
      ~isFocused=focus == Focus.SCM,
      state.scm,
    );

  let explorerContextKeys =
    Feature_Explorer.Contributions.contextKeys(
      ~isFocused=focus == Focus.FileExplorer,
      state.fileExplorer,
    );

  let extensionContextKeys =
    Feature_Extensions.Contributions.contextKeys(
      ~isFocused=focus == Focus.Extensions,
      state.extensions,
    );

  let searchContextKeys =
    Feature_Search.Contributions.contextKeys(
      ~isFocused=focus == Focus.Search,
      state.searchPane,
    );

  let paneContextKeys =
    Feature_Pane.Contributions.contextKeys(
      ~isFocused=focus == Focus.Pane,
      state.pane,
    );

  let isEditorFocused =
    switch (focus) {
    | Focus.Editor
    | Focus.Terminal(_) => true
    | _ => false
    };

  unionMany([
    Feature_Registers.Contributions.contextKeys(
      ~isFocused=focus == Focus.InsertRegister,
      state.registers,
    ),
    explorerContextKeys,
    sideBarContext,
    scmContextKeys,
    extensionContextKeys,
    searchContextKeys,
    paneContextKeys,
    Feature_LanguageSupport.Contributions.contextKeys
    |> Schema.map(({languageSupport, _}: State.t) => languageSupport)
    |> fromSchema(state),
    menus(~isFocused=focus == Focus.Quickmenu || focus == Focus.Wildmenu)
    |> Schema.map((state: State.t) => state.quickmenu)
    |> fromSchema(state),
    editors(~isFocused=isEditorFocused) |> fromSchema(state),
    other |> fromSchema(state),
  ]);
};
