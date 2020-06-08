type model = unit;

let initial = ();

[@deriving show]
type command =
  | FormatDocument;

[@deriving show]
type msg =
  | Command(command);

module Commands = {
  open Feature_Commands.Schema;

  let formatDocument =
    define(
      ~category="Formatting",
      ~title="Format Document",
      "editor.action.formatDocument",
      Command(FormatDocument),
    );
};

module Contributions = {
  let commands = [Commands.formatDocument];
};
