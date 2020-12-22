open Oni_Core;
open MenuBar.Schema;

module OSX = {
  let application =
    menu(~order=0, ~uniqueId="application", ~parent=None, "Application");
};

let isMac = Revery.Environment.os == Revery.Environment.Mac;

let file = menu(~order=100, ~uniqueId="file", ~parent=None, "File");

// The 'Application' menu on OSX
let application = isMac ? OSX.application : file;

let edit = menu(~order=200, ~uniqueId="edit", ~parent=None, "Edit");

let view = menu(~order=300, ~uniqueId="view", ~parent=None, "View");

let help = menu(~order=1000, ~uniqueId="help", ~parent=None, "Help");

module Items = {
  module File = {
    let newFile = item(~title="New", ~command=":enew");

    let saveFile = item(~title="Save", ~command=":w!");
    let saveAll = item(~title="Save All", ~command=":wa!");
    let quit = item(~title="Quit", ~command=":q");

    module Preferences = {
      let test1 = item(~title="Test1", ~command="test1");
      let test2 = item(~title="Test2", ~command="test2");
      let test3 = item(~title="Test3", ~command="test3");

      let submenu =
        submenu(
          ~title="Preferences",
          [group(~order=1, ~parent=file, [test1, test2, test3])],
        );
    };
  };
  module Edit = {
    let undo = item(~title="Undo", ~command="undo");
    let redo = item(~title="Redo", ~command="redo");
  };
  module Help = {
    let changelog = item(~title="Changelog", ~command="oni.changelog");
  };
};

let menus = (isMac ? [OSX.application] : []) @ [file, edit, view, help];

let groups = [
  group(~order=100, ~parent=file, Items.File.[newFile]),
  group(~order=200, ~parent=file, Items.File.[saveFile, saveAll]),
  group(~order=300, ~parent=file, Items.File.Preferences.[submenu]),
  group(~order=999, ~parent=file, Items.File.[quit]),
  group(~parent=edit, Items.Edit.[undo, redo]),
  group(~parent=help, Items.Help.[changelog]),
];
