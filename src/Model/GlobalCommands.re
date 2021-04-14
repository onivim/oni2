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

let noop = register("noop", Noop);

let command = cmd => CommandInvoked({command: cmd, arguments: `Null});

let undo = register("undo", command("undo"));
let redo = register("redo", command("redo"));

let indent = register("indent", command("indent"));
let outdent = register("outdent", command("outdent"));

module Editor = {
  module Action = {
    let indentLines =
      register(
        "editor.action.indentLines",
        command("editor.action.indentLines"),
      );

    let outdentLines =
      register(
        "editor.action.outdentLines",
        command("editor.action.outdentLines"),
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
      command("oni.changelog"),
    );

  module System = {
    let addToPath =
      register(
        ~category="System",
        ~title="Add Oni2 to System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && !symLinkExists"), // NOTE: symLinkExists only defined in command palette
        "system.addToPath",
        command("system.addToPath"),
      );

    let removeFromPath =
      register(
        ~category="System",
        ~title="Remove Oni2 from System PATH",
        ~isEnabledWhen=WhenExpr.parse("isMac && symLinkExists"), // NOTE: symLinkExists only defined in command palette
        "system.removeFromPath",
        command("system.removeFromPath"),
      );
  };

  module Vim = {
    let esc = register("vim.esc", command("vim.esc"));
    let tutor =
      register(
        ~category="Help",
        ~title="Open Vim Tutor",
        "vim.tutor",
        command("vim.tutor"),
      );
  };
};

module Workbench = {
  module Action = {
    let showCommands =
      register(
        ~title="Show All Commands",
        "workbench.action.showCommands",
        QuickmenuShow(CommandPalette),
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

    module Files = {
      let save =
        register(
          "workbench.action.files.save",
          command("workbench.action.files.save"),
        );
    };
  };
};

let registrations = () => Internal.registrations^;
