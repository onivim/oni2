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
};

let registrations = () => Internal.registrations^;
