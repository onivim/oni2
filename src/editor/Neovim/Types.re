open Oni_Core;
open Oni_Core.Types;

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
  type t = {
    activeBufferId: int,
    cursorLine: Index.t,
    cursorColumn: Index.t,
  };

  let create = (~activeBufferId, ~cursorLine, ~cursorColumn, ()) => {
    activeBufferId,
    cursorLine,
    cursorColumn,
  };

  let show = (v: t) => {
    "activeBufferId: "
    ++ string_of_int(v.activeBufferId)
    ++ " line: "
    ++ string_of_int(Index.toZeroBasedInt(v.cursorLine))
    ++ " column: "
    ++ string_of_int(Index.toZeroBasedInt(v.cursorColumn));
  };
};
