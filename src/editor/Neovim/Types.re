open Oni_Core;
open Oni_Core.Types;

module Commandline = Commandline;
module Wildmenu = Wildmenu;
module Tabline = Tabline;

module CursorPosition = {
  type t = {
    line: Index.t,
    character: Index.t,
  };
};

module EditorState = {
  type t = {
    activeBuffer: Buffer.t,
    cursorPosition: CursorPosition.t,
  };
};

module AutoCommandContext = {
  [@deriving show]
  type t = {
    activeBufferId: int,
    cursorLine: Index.t,
    cursorColumn: Index.t,
    modified: bool,
  };

  let create = (~activeBufferId, ~cursorLine, ~cursorColumn, ~modified) => {
    activeBufferId,
    cursorLine,
    cursorColumn,
    modified,
  };
};
