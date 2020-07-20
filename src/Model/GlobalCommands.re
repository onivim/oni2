open Actions;
open Feature_Commands.Schema;

module Internal = {
  let registrations = ref([]);

  let register = (~category=?, ~title=?, ~icon=?, ~isEnabledWhen=?, id, msg) => {
    let entry = define(~category?, ~title?, ~icon?, ~isEnabledWhen?, id, msg);
    registrations := [entry, ...registrations^];
    entry;
  };
};

open Internal;

let copyFilePath =
  register(
    ~category="Editor",
    ~title="Copy Active Filepath to Clipboard",
    "copyFilePath",
    CopyActiveFilepathToClipboard,
  );

let acceptSelectedSuggestion =
  register("acceptSelectedSuggestion", Command("acceptSelectedSuggestion"));

let selectPrevSuggestion =
  register("selectPrevSuggestion", Command("selectPrevSuggestion"));

let selectNextSuggestion =
  register("selectNextSuggestion", Command("selectNextSuggestion"));

let undo = register("undo", Command("undo"));
let redo = register("redo", Command("redo"));

let indent = register("indent", Command("indent"));
let outdent = register("outdent", Command("outdent"));

module Editor = {
  module Action = {
    let detectIndentation =
      register(
        ~category="Editor",
        ~title="Detect Indentation from Content",
        "editor.action.detectIndentation",
        Command("editor.action.detectIndentation"),
      );

    let indentLines =
      register(
        "editor.action.indentLines",
        Command("editor.action.indentLines"),
      );

    let outdentLines =
      register(
        "editor.action.outdentLines",
        Command("editor.action.outdentLines"),
      );
  };
};

module List = {
  let focusDown = register("list.focusDown", ListFocusDown);
  let focusUp = register("list.focusUp", ListFocusUp);

  let select = register("list.select", ListSelect);
  let selectBackground =
    register("list.selectBackground", ListSelectBackground);
};

module Oni = {
  let changelog =
    register(
      ~category="Help",
      ~title="Open changelog",
      "oni.changelog",
      Command("oni.changelog"),
    );

  module Explorer = {
    let toggle =
      register(
        ~category="View",
        ~title="Toggle File Explorer visibility",
        "explorer.toggle", // use workbench.action.toggleSidebarVisibility instead?
        Actions.ActivityBar(ActivityBar.FileExplorerClick),
      );
  };

  module KeyDisplayer = {
    let disable =
      register(
        ~category="Input",
        ~title="Disable Key Displayer",
        ~isEnabledWhen=WhenExpr.parse("keyDisplayerEnabled"),
        "keyDisplayer.disable",
        DisableKeyDisplayer,
      );

    let enable =
      register(
        ~category="Input",
        ~title="Enable Key Displayer",
        ~isEnabledWhen=WhenExpr.parse("!keyDisplayerEnabled"),
        "keyDisplayer.enable",
        EnableKeyDisplayer,
      );
  };

  module System = {
    let addToPath =
      register(
        ~category="System",
        ~title="Add Oni2 to System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && !symLinkExists"), // NOTE: symLinkExists only defined in command palette
        "system.addToPath",
        Command("system.addToPath"),
      );

    let removeFromPath =
      register(
        ~category="System",
        ~title="Remove Oni2 from System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && symLinkExists"), // NOTE: symLinkExists only defined in command palette
        "system.removeFromPath",
        Command("system.removeFromPath"),
      );
  };

  module Vim = {
    let esc = register("vim.esc", Command("vim.esc"));
    let tutor =
      register(
        ~category="Help",
        ~title="Open Vim Tutor",
        "vim.tutor",
        Command("vim.tutor"),
      );
  };

  module Workbench = {
    module Action = {
      let reloadSettings =
        register(
          ~category="Preferences",
          ~title="Reload configuration",
          "workbench.action.reloadSettings",
          ConfigurationReload,
        );

      let enableZenMode =
        register(
          ~category="View",
          ~title="Enable Zen Mode",
          ~isEnabledWhen=WhenExpr.parse("!zenMode"),
          "workbench.action.enableZenMode", // use workbench.action.toggleZenMode?
          //~isEnabled=() => !getState().zenMode,
          EnableZenMode,
        );

      let disableZenMode =
        register(
          ~category="View",
          ~title="Disable Zen Mode",
          ~isEnabledWhen=WhenExpr.parse("zenMode"),
          "workbench.action.disableZenMode", // use workbench.action.toggleZenMode?
          DisableZenMode,
        );
    };
  };
};

module ReferencesView = {
  let find =
    register(
      ~category="References",
      ~title="Find all References",
      "references-view.find",
      References(References.Requested),
    );
};

module Workbench = {
  module Action = {
    let openSettings =
      register(
        ~category="Preferences",
        ~title="Open configuration file",
        "workbench.action.openSettings",
        OpenConfigFile("configuration.json"),
      );

    let openDefaultKeybindingsFile =
      register(
        ~category="Preferences",
        ~title="Open keybindings file",
        "workbench.action.openDefaultKeybindingsFile",
        OpenConfigFile("keybindings.json"),
      );

    //    let selectTheme =
    //      register(
    //        ~category="Preferences",
    //        ~title="Theme Picker",
    //        "workbench.action.selectTheme",
    //        QuickmenuShow(ThemesPicker),
    //      );

    let showCommands =
      register(
        ~title="Show All Commands",
        "workbench.action.showCommands",
        QuickmenuShow(CommandPalette),
      );

    let gotoSymbol =
      register(
        ~title="Goto symbol in file...",
        "workbench.action.gotoSymbol",
        QuickmenuShow(DocumentSymbols),
      );

    let openNextRecentlyUsedEditorInGroup =
      register(
        ~category="View",
        ~title="Open Next Recently Used Editor In Group",
        "workbench.action.openNextRecentlyUsedEditorInGroup",
        QuickmenuShow(EditorsPicker),
      );

    let quickOpen =
      register(
        ~title="Go to File...",
        "workbench.action.quickOpen",
        QuickmenuShow(FilesPicker),
      );

    let quickOpenNavigateNextInEditorPicker =
      register(
        ~title="Navigate Next in Quick Open",
        "workbench.action.quickOpenNavigateNextInEditorPicker",
        ListFocusDown,
      );

    let quickOpenNavigatePreviousInEditorPicker =
      register(
        ~title="Navigate Previous in Quick Open",
        "workbench.action.quickOpenNavigatePreviousInEditorPicker",
        ListFocusUp,
      );

    let closeQuickOpen =
      register("workbench.action.closeQuickOpen", QuickmenuClose);

    let findInFiles =
      register(
        ~category="Search",
        ~title="Find in Files",
        "workbench.action.findInFiles",
        SearchHotkey,
      );

    let zoomIn =
      register(
        ~category="View",
        ~title="Zoom In",
        "workbench.action.zoomIn",
        Command("workbench.action.zoomIn"),
      );

    let zoomOut =
      register(
        ~category="View",
        ~title="Zoom Out",
        "workbench.action.zoomOut",
        Command("workbench.action.zoomOut"),
      );

    let zoomReset =
      register(
        ~category="View",
        ~title="Reset Zoom",
        "workbench.action.zoomReset",
        Command("workbench.action.zoomReset"),
      );

    module Files = {
      let save =
        register(
          "workbench.action.files.save",
          Command("workbench.action.files.save"),
        );
    };
  };
  module Actions = {
    module View = {
      let problems =
        register(
          ~category="View",
          ~title="Toggle Problems (Errors, Warnings)",
          "workbench.actions.view.problems",
          Command("workbench.actions.view.problems"),
        );
    };
  };
};

let registrations = () => Internal.registrations^;
