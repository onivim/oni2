open WhenExpr.ContextKeys.Schema;

let menus =
  fromList(
    Quickmenu.[
      bool("listFocus", model => model != None),
      bool("inQuickOpen", model => model != None),
      bool(
        "quickmenuCursorEnd",
        fun
        | Some({selection, query, _})
            when
              Selection.isCollapsed(selection)
              && selection.focus == String.length(query) =>
          true
        | _ => false,
      ),
      bool(
        "inEditorsPicker",
        fun
        | Some({variant: EditorsPicker, _}) => true
        | _ => false,
      ),
    ],
  );

let editors =
  fromList([
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
      | TerminalVisual
      | Visual => true
      | _ => false
      }
    ),
    bool("editorTextFocus", state =>
      switch (ModeManager.current(state)) {
      | TerminalInsert
      | TerminalNormal
      | TerminalVisual => false
      | _ => true
      }
    ),
    bool("terminalFocus", state =>
      switch (ModeManager.current(state)) {
      | TerminalInsert
      | TerminalNormal
      | TerminalVisual => true
      | _ => false
      }
    ),
    bool("commandLineFocus", state =>
      ModeManager.current(state) == CommandLine
    ),
  ]);

let other =
  fromList(
    State.[
      bool(
        "suggestWidgetVisible",
        fun
        | {completions, _} when Completions.isActive(completions) => true
        | _ => false,
      ),
      bool("sneakMode", state => Sneak.isActive(state.sneak)),
    ],
  );

let all =
  unionMany(State.[menus |> map(state => state.quickmenu), editors, other]);
