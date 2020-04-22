/*
 * BufferViewTokenizer.re
 */

open EditorCoreTypes;
open Revery;

open Oni_Core;

type tokenType =
  | Tab
  | Whitespace
  | Text;

type t = {
  tokenType,
  text: string,
  startPosition: Index.t,
  endPosition: Index.t,
  color: Color.t,
  backgroundColor: Color.t,
};

let space = Uchar.of_char(' ');
let tab = Uchar.of_char('\t');
let cr = Uchar.of_char('\r');
let lf = Uchar.of_char('\n');

let isWhitespace = c => {
  Uchar.equal(space, c)
  || Uchar.equal(tab, c)
  || Uchar.equal(cr, c)
  || Uchar.equal(lf, c);
};

let filterRuns = (r: Tokenizer.TextRun.t) => ZedBundled.length(r.text) != 0;

let textRunToToken = (colorizer, r: Tokenizer.TextRun.t) => {
  //let startIndex = Index.toZeroBased(r.startIndex);
  let (backgroundColor, color) = colorizer(r.startByte);

  let firstChar = ZedBundled.get(r.text, 0);

  let tokenType =
    if (Uchar.equal(firstChar, tab)) {
      Tab;
    } else if (Uchar.equal(firstChar, space)) {
      Whitespace;
    } else {
      Text;
    };

  {
    tokenType,
    text: r.text,
    startPosition: r.startPosition,
    endPosition: r.endPosition,
    color,
    backgroundColor,
  };
};

let getCharacterPositionAndWidth = (~viewOffset: int=0, line: BufferLine.t, i) => {
  let (totalOffset, width) = BufferLine.getPositionAndWidth(~index=i, line);

  let actualOffset =
    if (viewOffset > 0) {
      totalOffset - viewOffset;
    } else {
      totalOffset;
    };

  (actualOffset, width);
};

let tokenize = (~startIndex=0, ~endIndex, line, colorizer) => {
  let split =
      (
        ~index0 as _,
        ~byte0 as b0,
        ~char0 as c0,
        ~index1 as _,
        ~byte1 as b1,
        ~char1 as c1,
      ) => {
    let (bg0, fg0) = colorizer(b0);
    let (bg1, fg1) = colorizer(b1);

    !Color.equals(bg0, bg1)
    || !Color.equals(fg0, fg1)
    || isWhitespace(c0) != isWhitespace(c1)
    /* Always split on tabs */
    || Uchar.equal(c0, tab)
    || Uchar.equal(c1, tab);
  };

  Tokenizer.tokenize(~startIndex, ~endIndex, ~f=split, line)
  |> List.filter(filterRuns)
  |> List.map(textRunToToken(colorizer));
};
