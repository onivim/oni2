open Oni_Core;
open MenuBar.Schema;

// The 'Application' menu on OSX
let application =
  menu(~order=0, ~uniqueId="application", ~parent=None, "Application");

let file = menu(~order=1, ~uniqueId="file", ~parent=None, "File");

let edit = menu(~order=2, ~uniqueId="edit", ~parent=None, "Edit");

let view = menu(~order=3, ~uniqueId="view", ~parent=None, "View");
