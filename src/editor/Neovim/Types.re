open Oni_Core.Types;
open Oni_Model;

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
  [@deriving show({with_path: false})]
  type t = {
    activeBufferId: int,
    cursorLine: Index.t,
    cursorColumn: Index.t,
    modified: bool,
    visualRange: VisualRange.t,
  };

  let create =
      (
        ~activeBufferId,
        ~cursorLine,
        ~cursorColumn,
        ~modified,
        ~visualRange,
        (),
      ) => {
    activeBufferId,
    cursorLine,
    cursorColumn,
    modified,
    visualRange,
  };
};
