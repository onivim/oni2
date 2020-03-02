/*
 * BufferViewTokenizer.re
 */

open EditorCoreTypes;
open Revery;

open Oni_Core;

open CamomileLibrary;

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

let space = UChar.of_char(' ');
let tab = UChar.of_char('\t');
let cr = UChar.of_char('\r');
let lf = UChar.of_char('\n');

let isWhitespace = c => {
  UChar.eq(space, c)
  || UChar.eq(tab, c)
  || UChar.eq(cr, c)
  || UChar.eq(lf, c);
};

let filterRuns = (r: Tokenizer.TextRun.t) => ZedBundled.length(r.text) != 0;

let textRunToToken = (colorizer, r: Tokenizer.TextRun.t) => {
  let startIndex = Index.toZeroBased(r.startIndex);
  let (backgroundColor, color) = colorizer(startIndex);

  let firstChar = ZedBundled.get(r.text, 0);

  let tokenType =
    if (UChar.eq(firstChar, tab)) {
      Tab;
    } else if (UChar.eq(firstChar, space)) {
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
  let split = (i0, c0, i1, c1) => {
    let (bg0, fg0) = colorizer(i0);
    let (bg1, fg1) = colorizer(i1);

    !Color.equals(bg0, bg1)
    || !Color.equals(fg0, fg1)
    || isWhitespace(c0) != isWhitespace(c1)
    /* Always split on tabs */
    || UChar.eq(c0, tab)
    || UChar.eq(c1, tab);
  };

  Tokenizer.tokenize(~startIndex, ~endIndex, ~f=split, line)
  |> List.filter(filterRuns)
  |> List.map(textRunToToken(colorizer));
};
