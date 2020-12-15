open Oni_Core;
open MenuBar.Schema;

// The 'Application' menu on OSX
let application =
  menu(~uniqueId="application", ~title="Application", ~parent=None);

let file = menu(~uniqueId="file", ~title="File", ~parent=None);

let edit = menu(~uniqueId="edit", ~title="Edit", ~parent=None);

let view = menu(~uniqueId="view", ~title="View", ~parent=None);
