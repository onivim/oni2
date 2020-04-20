open Actions;
open Feature_Commands.Schema;

let copyFilePath =
  define(
    ~category="Editor",
    ~title="Copy Active Filepath to Clipboard",
    "copyFilePath",
    CopyActiveFilepathToClipboard,
  );

let acceptSelectedSuggestion =
  define("acceptSelectedSuggestion", Command("acceptSelectedSuggestion"));

let selectPrevSuggestion =
  define("selectPrevSuggestion", Command("selectPrevSuggestion"));

let selectNextSuggestion =
  define("selectNextSuggestion", Command("selectNextSuggestion"));

let undo = define("undo", Command("undo"));
let redo = define("redo", Command("redo"));

let indent = define("indent", Command("indent"));
let outdent = define("outdent", Command("outdent"));

module Editor = {
  module Action = {
    let detectIndentation =
      define(
        ~category="Editor",
        ~title="Detect Indentation from Content",
        "editor.action.detectIndentation",
        Command("editor.action.detectIndentation"),
      );

    let clipboardPasteAction =
      define(
        "editor.action.clipboardPasteAction",
        Command("editor.action.clipboardPasteAction"),
      );

    let indentLines =
      define(
        "editor.action.indentLines",
        Command("editor.action.indentLines"),
      );

    let outdentLines =
      define(
        "editor.action.outdentLines",
        Command("editor.action.outdentLines"),
      );
  };
};

module List = {
  let focusDown = define("list.focusDown", ListFocusDown);
  let focusUp = define("list.focusUp", ListFocusUp);

  let select = define("list.select", ListSelect);
  let selectBackground =
    define("list.selectBackground", ListSelectBackground);
};

module Oni = {
  module Explorer = {
    let toggle =
      define(
        ~category="View",
        ~title="Toggle File Explorer visibility",
        "oni.explorer.toggle", // use workbench.action.toggleSidebarVisibility instead?
        Actions.ActivityBar(ActivityBar.FileExplorerClick),
      );
  };

  module KeyDisplayer = {
    let disable =
      define(
        ~category="Input",
        ~title="Disable Key Displayer",
        ~isEnabledWhen=WhenExpr.parse("oni.keyDisplayerEnabled"),
        "oni.keyDisplayer.disable",
        DisableKeyDisplayer,
      );

    let enable =
      define(
        ~category="Input",
        ~title="Enable Key Displayer",
        ~isEnabledWhen=WhenExpr.parse("!oni.keyDisplayerEnabled"),
        "oni.keyDisplayer.enable",
        EnableKeyDisplayer,
      );
  };

  module Sneak = {
    let start =
      define(
        ~category="Sneak",
        ~title="Enter sneak mode (keyboard-accessible UI)",
        "oni.sneak.start",
        Command("sneak.start"),
      );

    let stop =
      define(
        ~category="Sneak",
        ~title="Exit sneak mode",
        "oni.sneak.stop",
        Command("sneak.stop"),
      );
  };

  module System = {
    let addToPath =
      define(
        ~category="System",
        ~title="Add Oni2 to System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && !oni.symLinkExists"),
        "oni.system.addToPath",
        Command("system.addToPath"),
      );

    let removeFromPath =
      define(
        ~category="System",
        ~title="Remove Oni2 from System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && oni.symLinkExists"),
        "oni.system.removeFromPath",
        Command("system.removeFromPath"),
      );
  };

  module View = {
    let rotateForward =
      define(
        ~category="View",
        ~title="Rotate Windows (Forwards)",
        "oni.view.rotateForward",
        Command("view.rotateForward"),
      );

    let rotateBackward =
      define(
        ~category="View",
        ~title="Rotate Windows (Backwards)",
        "oni.view.rotateBackward",
        Command("view.rotateBackward"),
      );
  };

  module Vim = {
    let esc = define("oni.vim.esc", Command("vim.esc"));
  };

  module Workbench = {
    module Action = {
      let reloadSettings =
        define(
          ~category="Preferences",
          ~title="Reload configuration",
          "oni.workbench.action.reloadSettings",
          ConfigurationReload,
        );

      let enableZenMode =
        define(
          ~category="View",
          ~title="Enable Zen Mode",
          ~isEnabledWhen=WhenExpr.parse("!zenMode"),
          "oni.workbench.action.enableZenMode", // use workbench.action.toggleZenMode?
          //~isEnabled=() => !getState().zenMode,
          EnableZenMode,
        );

      let disableZenMode =
        define(
          ~category="View",
          ~title="Disable Zen Mode",
          ~isEnabledWhen=WhenExpr.parse("zenMode"),
          "oni.workbench.action.disableZenMode", // use workbench.action.toggleZenMode?
          DisableZenMode,
        );
    };
  };
};

module ReferencesView = {
  let find =
    define(
      ~category="References",
      ~title="Find all References",
      "references-view.find",
      References(References.Requested),
    );
};

module View = {
  let closeEditor =
    define(
      ~category="View",
      ~title="Close Editor",
      "view.closeEditor",
      Command("view.closeEditor"),
    );

  let splitVertical =
    define(
      ~category="View",
      ~title="Split Editor Vertically",
      "view.splitVertical",
      Command("view.splitVertical"),
    );

  let splitHorizontal =
    define(
      ~category="View",
      ~title="Split Editor Horizontally",
      "view.splitHorizontal",
      Command("view.splitHorizontal"),
    );
};

module Workbench = {
  module Action = {
    let openSettings =
      define(
        ~category="Preferences",
        ~title="Open configuration file",
        "workbench.action.openSettings",
        OpenConfigFile("configuration.json"),
      );

    let openDefaultKeybindingsFile =
      define(
        ~category="Preferences",
        ~title="Open keybindings file",
        "workbench.action.openDefaultKeybindingsFile",
        OpenConfigFile("keybindings.json"),
      );

    let selectTheme =
      define(
        ~category="Preferences",
        ~title="Theme Picker",
        "workbench.action.selectTheme",
        QuickmenuShow(ThemesPicker),
      );

    let showCommands =
      define(
        ~title="Show All Commands",
        "workbench.action.showCommands",
        QuickmenuShow(CommandPalette),
      );

    let gotoSymbol =
      define(
        ~title="Goto symbol in file...",
        "workbench.action.gotoSymbol",
        QuickmenuShow(DocumentSymbols),
      );

    let openNextRecentlyUsedEditorInGroup =
      define(
        ~category="View",
        ~title="Open Next Recently Used Editor In Group",
        "workbench.action.openNextRecentlyUsedEditorInGroup",
        QuickmenuShow(EditorsPicker),
      );

    let quickOpen =
      define(
        ~title="Go to File...",
        "workbench.action.quickOpen",
        QuickmenuShow(FilesPicker),
      );

    let nextEditor =
      define(
        ~category="View",
        ~title="Open Next Editor",
        "workbench.action.nextEditor",
        Command("workbench.action.nextEditor"),
      );

    let previousEditor =
      define(
        ~category="View",
        ~title="Open Previous Editor",
        "workbench.action.previousEditor",
        Command("workbench.action.previousEditor"),
      );

    let quickOpenNavigateNextInEditorPicker =
      define(
        ~title="Navigate Next in Quick Open",
        "workbench.action.quickOpenNavigateNextInEditorPicker",
        ListFocusDown,
      );

    let quickOpenNavigatePreviousInEditorPicker =
      define(
        ~title="Navigate Previous in Quick Open",
        "workbench.action.quickOpenNavigatePreviousInEditorPicker",
        ListFocusUp,
      );

    let closeQuickOpen =
      define("workbench.action.closeQuickOpen", QuickmenuClose);

    let findInFiles =
      define(
        ~category="Search",
        ~title="Find in Files",
        "workbench.action.findInFiles",
        SearchHotkey,
      );

    module Files = {
      let save =
        define("workbench.action.save", Command("workbench.action.save"));
    };
  };
  module Actions = {
    module View = {
      let problems =
        define(
          ~category="View",
          ~title="Toggle Problems (Errors, Warnings)",
          "workbench.actions.view.problems",
          Command("workbench.actions.view.problems"),
        );
    };
  };
};

let contributions = [
  copyFilePath,
  acceptSelectedSuggestion,
  selectPrevSuggestion,
  selectNextSuggestion,
  undo,
  redo,
  indent,
  outdent,
  Editor.Action.detectIndentation,
  Editor.Action.clipboardPasteAction,
  Editor.Action.indentLines,
  Editor.Action.outdentLines,
  List.focusDown,
  List.focusUp,
  List.select,
  List.selectBackground,
  Oni.Explorer.toggle,
  Oni.KeyDisplayer.disable,
  Oni.KeyDisplayer.enable,
  Oni.Sneak.start,
  Oni.Sneak.stop,
  Oni.System.addToPath,
  Oni.System.removeFromPath,
  Oni.View.rotateForward,
  Oni.View.rotateBackward,
  Oni.Vim.esc,
  Oni.Workbench.Action.enableZenMode,
  Oni.Workbench.Action.disableZenMode,
  Oni.Workbench.Action.reloadSettings,
  ReferencesView.find,
  View.closeEditor,
  View.splitVertical,
  View.splitHorizontal,
  Workbench.Action.openSettings,
  Workbench.Action.openDefaultKeybindingsFile,
  Workbench.Action.selectTheme,
  Workbench.Action.showCommands,
  Workbench.Action.gotoSymbol,
  Workbench.Action.openNextRecentlyUsedEditorInGroup,
  Workbench.Action.quickOpen,
  Workbench.Action.nextEditor,
  Workbench.Action.previousEditor,
  Workbench.Action.findInFiles,
  Workbench.Action.quickOpenNavigateNextInEditorPicker,
  Workbench.Action.quickOpenNavigatePreviousInEditorPicker,
  Workbench.Action.closeQuickOpen,
  Workbench.Action.nextEditor,
  Workbench.Action.previousEditor,
  Workbench.Action.Files.save,
  Workbench.Actions.View.problems,
];
