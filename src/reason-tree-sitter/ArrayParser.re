/*
     ArrayParser.re

     Specialized parser for arrays of string
 */

type t;

module Baseline = {
  type t = {
    lengths: array(int),
    tree: Tree.t,
  };

  let create = (~lengths, ~tree, ()) => {lengths, tree};
};

module Delta = {
  type t = {
    tree: Tree.t,
    startLine: int,
    oldEndLine: int,
    oldOffsets: array(int),
    newLines: array(string),
  };

  let getOffsetToLine = (startLine, endLine, lines: array(int)) => {
    let i = ref(startLine);
    let offset = ref(0);

    while (i^ < endLine) {
      offset := offset^ + lines[i^];
      incr(i);
    };

    offset^;
  };

  let getOffsetToLineStr = (startLine, endLine, lines: array(string)) => {
    let i = ref(startLine);
    let offset = ref(0);

    while (i^ < endLine) {
      offset := offset^ + String.length(lines[i^]) + 1;
      incr(i);
    };

    offset^;
  };

  let create =
      (
        baseline: Baseline.t,
        startLine: int,
        oldEndLine: int,
        newLines: array(string),
      ) => {
    let {lengths, tree}: Baseline.t = baseline;
    let startByte = getOffsetToLine(0, startLine, lengths);
    let oldEndByte = getOffsetToLine(0, oldEndLine, lengths);

    let len = Array.length(newLines);
    let newEndByte = startByte + getOffsetToLineStr(0, len, newLines);
    let newEndLine = startLine + len;

    let newTree =
      Tree.edit(
        tree,
        startByte,
        oldEndByte,
        newEndByte,
        startLine,
        oldEndLine,
        newEndLine,
      );
    {
      tree: newTree,
      startLine,
      oldEndLine,
      newLines,
      oldOffsets: baseline.lengths,
    };
  };
};

let parse = (parser: Parser.t, delta: option(Delta.t), lines: array(string)) => {
  let len = Array.length(lines);
  let byteOffsets: array(int) = Array.make(len, 0);

  // The interop between C <-> OCaml is expensive for large files.
  // We should look to see if we can instead access the array directly
  // from the C side.
  let f = (_byteOffset, line, col) =>
    if (line < len) {
      let v = lines[line] ++ "\n";
      let strlen = String.length(v);

      if (col < strlen) {
        let ret = String.sub(v, col, strlen - col);
        Some(ret);
      } else {
        None;
      };
    } else {
      None;
    };

  // TODO: Copy over byte offsets from previous baseline / delta
  let i = ref(0);
  while (i^ < len) {
    let idx = i^;
    byteOffsets[idx] = String.length(lines[idx]) + 1;
    incr(i);
  };

  let oldTree =
    switch (delta) {
    | Some({tree, _}) => Some(tree)
    | None => None
    };

  let tree = Parser.parse(parser, oldTree, f);
  let baseline = Baseline.create(~tree, ~lengths=byteOffsets, ());
  (tree, baseline);
};
