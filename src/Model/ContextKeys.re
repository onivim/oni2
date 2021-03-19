open WhenExpr.ContextKeys;

let menus = (~isFocused, ~isNewQuickMenuOpen) => {
  Schema.(
    fromList(
      // TODO: This should be factored to a feature...
      isFocused || isNewQuickMenuOpen
        ? Quickmenu.[
            bool(
              "commandLineFocus",
              fun
              | Some({variant: Wildmenu(_), _}) => true
              | _ => false,
            ),
            bool("listFocus", model => isNewQuickMenuOpen || model != None),
            bool("inQuickOpen", model => isNewQuickMenuOpen || model != None),
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
              | None => isNewQuickMenuOpen,
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
        ? [
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
            | Insert(_) => true
            | _ => false
            }
          ),
          bool("normalMode", state =>
            switch (ModeManager.current(state)) {
            | TerminalNormal
            | Normal(_) => true
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
          bool("selectMode", state =>
            switch (ModeManager.current(state)) {
            | Select(_) => true
            | _ => false
            }
          ),
          bool("operatorPending", state =>
            switch (ModeManager.current(state)) {
            | Operator(_) => true
            | _ => false
            }
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
        bool("isLinux", _state => Revery.Environment.isLinux),
        bool("isMac", _state => Revery.Environment.isMac),
        bool("isWin", _state => Revery.Environment.isWindows),
        bool("sneakMode", state => Feature_Sneak.isActive(state.sneak)),
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

  let inputContextKeys = Feature_Input.Contributions.contextKeys(state.input);

  let zenContextKeys = Feature_Zen.Contributions.contextKeys(state.zen);
  // let newQuickmenuContextKeys =
  //   Feature_Quickmenu.Contributions.contextKeys(state.newQuickmenu);

  unionMany([
    Feature_Registers.Contributions.contextKeys(
      ~isFocused=focus == Focus.InsertRegister,
      state.registers,
    ),
    inputContextKeys,
    explorerContextKeys,
    sideBarContext,
    scmContextKeys,
    extensionContextKeys,
    searchContextKeys,
    paneContextKeys,
    Feature_Registration.Contributions.contextKeys(
      ~isFocused=focus == Focus.LicenseKey,
      state.registration,
    ),
    Feature_LanguageSupport.Contributions.contextKeys
    |> Schema.map(({languageSupport, _}: State.t) => languageSupport)
    |> fromSchema(state),
    menus(
      ~isFocused=focus == Focus.Quickmenu || focus == Focus.Wildmenu,
      ~isNewQuickMenuOpen=focus == Focus.NewQuickmenu,
    )
    |> Schema.map((state: State.t) => state.quickmenu)
    |> fromSchema(state),
    editors(~isFocused=isEditorFocused) |> fromSchema(state),
    zenContextKeys,
    //newQuickmenuContextKeys,
    Feature_Snippets.Contributions.contextKeys(state.snippets),
    other |> fromSchema(state),
  ]);
};
