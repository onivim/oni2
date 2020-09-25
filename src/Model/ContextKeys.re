open WhenExpr.ContextKeys.Schema;

let menus = (~isFocused) => {
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
  );
};

let editors = (~isFocused) =>
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
  );

let other =
  fromList(
    State.[
      bool("isLinux", _state =>
        Revery.Environment.os == Revery.Environment.Linux
      ),
      bool("isMac", _state => Revery.Environment.os == Revery.Environment.Mac),
      bool("isWin", _state =>
        Revery.Environment.os == Revery.Environment.Windows
      ),
      bool("sneakMode", state => Feature_Sneak.isActive(state.sneak)),
      bool("zenMode", state => state.zenMode),
      bool("keyDisplayerEnabled", state => state.keyDisplayer != None),
    ],
  );

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
    );

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
    );

  let extensionContextKeys =
    Feature_Extensions.Contributions.contextKeys(
      ~isFocused=focus == Focus.Extensions,
      state.extensions,
    );

  let searchContextKeys =
    Feature_Search.Contributions.contextKeys(
      ~isFocused=focus == Focus.Search,
    );

  let paneContextKeys =
    Feature_Pane.Contributions.contextKeys(
      ~isFocused=focus == Focus.Pane,
      state.pane,
    );

  unionMany([
    Feature_Registers.Contributions.contextKeys(
      ~isFocused=focus == Focus.InsertRegister,
    )
    |> map(({registers, _}: State.t) => registers),
    explorerContextKeys |> map(({fileExplorer, _}: State.t) => fileExplorer),
    sideBarContext |> fromList |> map(({sideBar, _}: State.t) => sideBar),
    scmContextKeys |> map(({scm, _}: State.t) => scm),
    extensionContextKeys |> map(({extensions, _}: State.t) => extensions),
    searchContextKeys |> map(({searchPane, _}: State.t) => searchPane),
    paneContextKeys |> map(({pane, _}: State.t) => pane),
    Feature_LanguageSupport.Contributions.contextKeys
    |> map(({languageSupport, _}: State.t) => languageSupport),
    menus(~isFocused=focus == Focus.Quickmenu || focus == Focus.Wildmenu)
    |> map((state: State.t) => state.quickmenu),
    editors(~isFocused=focus == Focus.Editor),
    other,
  ]);
};
