open Actions;

let all =
  Feature_Commands.Schema.[
    define(
      ~category="Preferences",
      ~title="Open configuration file",
      "workbench.action.openSettings",
      OpenConfigFile("configuration.json"),
    ),
    define(
      ~category="Preferences",
      ~title="Open keybindings file",
      "workbench.action.openDefaultKeybindingsFile",
      OpenConfigFile("keybindings.json"),
    ),
    define(
      ~category="Preferences",
      ~title="Reload configuration",
      "oni.workbench.action.reloadSettings",
      ConfigurationReload,
    ),
    define(
      ~category="Preferences",
      ~title="Theme Picker",
      "workbench.action.selectTheme",
      QuickmenuShow(ThemesPicker),
    ),
    define(
      ~title="Show All Commands",
      "workbench.action.showCommands",
      QuickmenuShow(CommandPalette),
    ),
    define(
      ~title="Goto symbol in file...",
      "workbench.action.gotoSymbol",
      QuickmenuShow(DocumentSymbols),
    ),
    define(
      ~category="View",
      ~title="Open Next Recently Used Editor In Group",
      "workbench.action.openNextRecentlyUsedEditorInGroup",
      QuickmenuShow(EditorsPicker),
    ),
    define(
      ~title="Go to File...",
      "workbench.action.quickOpen",
      QuickmenuShow(FilesPicker),
    ),
    define(
      ~category="View",
      ~title="Close Editor",
      "view.closeEditor",
      Command("view.closeEditor"),
    ),
    define(
      ~category="View",
      ~title="Open Next Editor",
      "workbench.action.nextEditor",
      Command("workbench.action.nextEditor"),
    ),
    define(
      ~category="View",
      ~title="Open Previous Editor",
      "workbench.action.previousEditor",
      Command("workbench.action.previousEditor"),
    ),
    define(
      ~category="View",
      ~title="Toggle Problems (Errors, Warnings)",
      "workbench.actions.view.problems",
      Command("workbench.actions.view.problems"),
    ),
    define(
      ~category="View",
      ~title="Split Editor Vertically",
      "view.splitVertical",
      Command("view.splitVertical"),
    ),
    define(
      ~category="View",
      ~title="Split Editor Horizontally",
      "view.splitHorizontal",
      Command("view.splitHorizontal"),
    ),
    define(
      ~category="View",
      ~title="Enable Zen Mode",
      "oni.workbench.action.enableZenMode", // use workbench.action.toggleZenMode?
      //~isEnabled=() => !getState().zenMode,
      EnableZenMode,
    ),
    define(
      ~category="View",
      ~title="Disable Zen Mode",
      "oni.workbench.action.disableZenMode", // use workbench.action.toggleZenMode?
      //~isEnabled=() => getState().zenMode,
      DisableZenMode,
    ),
    define(
      ~category="Input",
      ~title="Disable Key Displayer",
      "oni.keyDisplayer.disable",
      //~isEnabled=() => getState().keyDisplayer != None,
      DisableKeyDisplayer,
    ),
    define(
      ~category="Input",
      ~title="Enable Key Displayer",
      "oni.keyDisplayer.enable",
      //~isEnabled=() => getState().keyDisplayer == None,
      EnableKeyDisplayer,
    ),
    define(
      ~category="References",
      ~title="Find all References",
      "references-view.find",
      References(References.Requested),
    ),
    define(
      ~category="View",
      ~title="Rotate Windows (Forwards)",
      "oni.view.rotateForward",
      Command("view.rotateForward"),
    ),
    define(
      ~category="View",
      ~title="Rotate Windows (Backwards)",
      "oni.view.rotateBackward",
      Command("view.rotateBackward"),
    ),
    define(
      ~category="Editor",
      ~title="Copy Active Filepath to Clipboard",
      "copyFilePath",
      CopyActiveFilepathToClipboard,
    ),
    define(
      ~category="Editor",
      ~title="Detect Indentation from Content",
      "editor.action.detectIndentation",
      Command("editor.action.detectIndentation"),
    ),
    define(
      ~category="System",
      ~title="Add Oni2 to System PATH",
      "oni.system.addToPath",
      //~isEnabled=() => pathSymlinkEnabled(~addingLink=true),
      Command("system.addToPath"),
    ),
    define(
      ~category="System",
      ~title="Remove Oni2 from System PATH",
      "oni.system.removeFromPath",
      //~isEnabled=() => pathSymlinkEnabled(~addingLink=false),
      Command("system.removeFromPath"),
    ),
    define(
      ~category="Sneak",
      ~title="Start sneak (keyboard-accessible UI)",
      "oni.sneak.start",
      Command("sneak.start"),
    ),
    define(
      ~category="Sneak",
      ~title="Stop sneak (keyboard-accessible UI)",
      "oni.sneak.stop",
      Command("sneak.stop"),
    ),
    define(
      ~category="Terminal",
      ~title="Open terminal in new horizontal split",
      "terminal.new.horizontal",
      Actions.Terminal(
        Feature_Terminal.NewTerminal({cmd: None, splitDirection: Horizontal}),
      ),
    ),
    define(
      ~category="Terminal",
      ~title="Open terminal in new vertical split",
      "terminal.new.vertical",
      Actions.Terminal(
        Feature_Terminal.NewTerminal({cmd: None, splitDirection: Vertical}),
      ),
    ),
    define(
      ~category="Terminal",
      ~title="Open terminal in current window",
      "terminal.new.current",
      Actions.Terminal(
        Feature_Terminal.NewTerminal({cmd: None, splitDirection: Current}),
      ),
    ),
    define(
      ~category="Search",
      ~title="Find in Files",
      "workbench.action.findInFiles",
      SearchHotkey,
    ),
    define(
      ~title="Navigate Next in Quick Open",
      "workbench.action.quickOpenNavigateNextInEditorPicker",
      ListFocusDown,
    ),
    define(
      ~title="Navigate Previous in Quick Open",
      "workbench.action.quickOpenNavigatePreviousInEditorPicker",
      ListFocusUp,
    ),
    define(
      ~category="View",
      ~title="Toggle File Explorer visibility",
      "oni.explorer.toggle", // use workbench.action.toggleSidebarVisibility instead?
      Actions.ActivityBar(ActivityBar.FileExplorerClick),
    ),
    define("list.focusDown", ListFocusDown),
    define("list.focusUp", ListFocusUp),
    define("list.select", ListSelect),
    define("list.selectBackground", ListSelectBackground),
    define("workbench.action.closeQuickOpen", QuickmenuClose),
    define("oni.terminal.normalMode", Command("terminal.normalMode")),
    define("oni.terminal.insertMode", Command("terminal.insertMode")),
    define("acceptSuggestedSuggestion", Command("acceptSelectedSuggestion")),
    define("selectPrevSuggestion", Command("selectPrevSuggestion")),
    define("selectNextSuggestion", Command("selectNextSuggestion")),
    define(
      "editor.action.clipboardPasteAction",
      Command("editor.action.clipboardPasteAction"),
    ),
    define("undo", Command("undo")),
    define("redo", Command("redo")),
    define("workbench.action.save", Command("workbench.action.save")),
    define("indent", Command("indent")),
    define("outdent", Command("outdent")),
    define(
      "editor.action.indentLines",
      Command("editor.action.indentLines"),
    ),
    define(
      "editor.action.outdentLines",
      Command("editor.action.outdentLines"),
    ),
    define("oni.vim.esc", Command("vim.esc")),
    define(
      "workbench.action.nextEditor",
      Command("workbench.action.nextEditor"),
    ),
    define(
      "workbench.action.previousEditor",
      Command("workbench.action.previousEditor"),
    ),
  ];
