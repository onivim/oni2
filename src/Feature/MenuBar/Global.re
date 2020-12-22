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
  group(~order=999, ~parent=file, Items.File.[quit]),
  group(~parent=edit, Items.Edit.[undo, redo]),
  group(~parent=help, Items.Help.[changelog]),
];
