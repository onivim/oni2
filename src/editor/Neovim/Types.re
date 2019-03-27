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

module VisualRange = {
  [@deriving show({with_path: false})]
  type mode =
    | None
    | Visual /* "v" */
    | BlockwiseVisual /* "<C-v>" */
    | LinewiseVisual; /* "V" */

  [@deriving show({with_path: false})]
  type t = {
    startLine: Index.t,
    startColumn: Index.t,
    endLine: Index.t,
    endColumn: Index.t,
    mode,
  };

  let _modeFromString = s => {
    switch (s) {
    | "V" => LinewiseVisual
    | "bv" => BlockwiseVisual
    | "v" => Visual
    | _ => None
    };
  };

  /*
   * The range might not always come through in the correct 'order' -
   * this method normalizes the range so that the (startLine, startColumn) is
   * before or equal to (endLine, endColumn)
   */
  let _normalizeRange = (startLine, startColumn, endLine, endColumn) =>
    if (startLine > endLine) {
      (endLine, endColumn, startLine, startColumn);
    } else if (startLine == endLine && startColumn > endColumn) {
      (endLine, endColumn, startLine, startColumn);
    } else {
      (startLine, startColumn, endLine, endColumn);
    };

  let create = (~startLine, ~startColumn, ~endLine, ~endColumn, ~mode, ()) => {
    let (startLine, startColumn, endLine, endColumn) =
      _normalizeRange(startLine, startColumn, endLine, endColumn);
    let mode = _modeFromString(mode);

    {
      startLine: OneBasedIndex(startLine),
      startColumn: OneBasedIndex(startColumn),
      endLine: OneBasedIndex(endLine),
      endColumn: OneBasedIndex(endColumn),
      mode,
    };
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
