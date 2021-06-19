open Oni_Core;
open ContextMenu.Schema;

module OSX = {
  let application =
    menu(~order=0, ~uniqueId="application", ~parent=None, "Application");
};

let file = menu(~order=100, ~uniqueId="file", ~parent=None, "File");

// The 'Application' menu on OSX
let application = Revery.Environment.isMac ? OSX.application : file;

let edit = menu(~order=200, ~uniqueId="edit", ~parent=None, "Edit");

let go = menu(~order=250, ~uniqueId="go", ~parent=None, "Go");

let view = menu(~order=300, ~uniqueId="view", ~parent=None, "View");

let help = menu(~order=1000, ~uniqueId="help", ~parent=None, "Help");

module Items = {
  module File = {
    let newFile = item(~title="New", ~command=":enew");

    let saveFile = item(~title="Save", ~command=":w!");
    let saveAll = item(~title="Save All", ~command=":wa!");
    let quit = item(~title="Quit", ~command=":qa");

    module Preferences = {
      let configuration =
        item(
          ~title="Configuration",
          ~command="workbench.action.openSettings",
        );
      let keybindings =
        item(
          ~title="Keybindings",
          ~command="workbench.action.openDefaultKeybindingsFile",
        );
      let selectTheme =
        item(~title="Change Theme", ~command="workbench.action.selectTheme");

      let userSnippets =
        item(
          ~title="User Snippets",
          ~command="workbench.action.openSnippets",
        );

      let submenu =
        submenu(
          ~title="Preferences",
          [
            group(
              ~order=1,
              ~parent=file,
              [configuration, keybindings, selectTheme],
            ),
            group(~order=2, ~parent=file, [userSnippets]),
          ],
        );
    };
  };
  module Edit = {
    let undo = item(~title="Undo", ~command="undo");
    let redo = item(~title="Redo", ~command="redo");
  };
  module Help = {
    let changelog = item(~title="Changelog", ~command="oni.changelog");

    let checkForUpdate =
      item(~title="Check for updates...", ~command="oni.app.checkForUpdates");

    let enterLicenseKey =
      item(~title="Enter license key", ~command="oni.app.enterLicenseKey");
  };

  module Go = {
    let gotoFile =
      item(~title="Go to File", ~command="workbench.action.quickOpen");
    let gotoBuffer =
      item(
        ~title="Go to Buffer",
        ~command="workbench.action.quickOpenBuffer",
      );
  };

  module View = {
    let commands =
      item(~title="Commands", ~command="workbench.action.showCommands");

    let cycleOpenFiles =
      item(
        ~title="Cycle open files",
        ~command="workbench.action.openNextRecentlyUsedEditorInGroup",
      );

    module Zoom = {
      let zoomIn = item(~title="Zoom in", ~command="workbench.action.zoomIn");

      let zoomOut =
        item(~title="Zoom out", ~command="workbench.action.zoomOut");

      let zoomReset =
        item(~title="Reset zoom", ~command="workbench.action.zoomReset");

      let submenu =
        submenu(
          ~title="Zoom",
          [group(~parent=view, [zoomIn, zoomOut, zoomReset])],
        );
    };
  };
};

let menus =
  (Revery.Environment.isMac ? [OSX.application] : [])
  @ [file, edit, go, view, help];

let groups = [
  group(~order=100, ~parent=file, Items.File.[newFile]),
  group(~order=200, ~parent=file, Items.File.[saveFile, saveAll]),
  group(~order=300, ~parent=application, Items.File.Preferences.[submenu]),
  group(~order=999, ~parent=file, Items.File.[quit]),
  group(~order=200, ~parent=go, Items.Go.[gotoFile, gotoBuffer]),
  group(~order=100, ~parent=view, Items.View.[commands, cycleOpenFiles]),
  group(~order=150, ~parent=view, Items.View.Zoom.[submenu]),
  group(~parent=edit, Items.Edit.[undo, redo]),
  group(
    ~parent=help,
    Items.Help.[changelog, enterLicenseKey, checkForUpdate],
  ),
];
