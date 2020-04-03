open Oni_Components;
open WhenExpr.ContextKeys.Schema;

let keys =
  fromList(
    State.[
      bool("listFocus", state => state.quickmenu != None),
      bool("inQuickOpen", state => state.quickmenu != None),
      bool(
        "quickmenuCursorEnd",
        fun
        | {quickmenu: Some({selection, query, _}), _}
            when
              Selection.isCollapsed(selection)
              && selection.focus == String.length(query) =>
          true
        | _ => false,
      ),
      bool(
        "inEditorsPicker",
        fun
        | {quickmenu: Some({variant: EditorsPicker, _}), _} => true
        | _ => false,
      ),
      bool(
        "suggestWidgetVisible",
        fun
        | {completions, _} when Completions.isActive(completions) => true
        | _ => false,
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
      bool("sneakMode", state => Sneak.isActive(state.sneak)),
    ],
  );
