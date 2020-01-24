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

let _isWhitespace = c => {
  UChar.eq(space, c)
  || UChar.eq(tab, c)
  || UChar.eq(cr, c)
  || UChar.eq(lf, c);
};

let _isNonWhitespace = c => !_isWhitespace(c);

let filterRuns = (r: Tokenizer.TextRun.t) => {
  let len = ZedBundled.length(r.text);

  if (len == 0) {
    false;
  } else {
    true;
  };
};

type colorizer = int => (Color.t, Color.t);

let textRunToToken = (colorizer: colorizer, r: Tokenizer.TextRun.t) => {
  let startIndex = Index.toZeroBased(r.startIndex);
  let (bg, fg) = colorizer(startIndex);

  let firstChar = ZedBundled.get(r.text, 0);

  let tokenType =
    if (UChar.eq(firstChar, tab)) {
      Tab;
    } else if (UChar.eq(firstChar, space)) {
      Whitespace;
    } else {
      Text;
    };

  let color = fg;
  let backgroundColor = bg;

  let ret: t = {
    tokenType,
    text: r.text,
    startPosition: r.startPosition,
    endPosition: r.endPosition,
    color,
    backgroundColor,
  };
  ret;
};

// TODO: Remove duplication between here and BufferLine
let measure = (indentationSettings: IndentationSettings.t, c) =>
  if (UChar.eq(c, tab)) {
    indentationSettings.tabSize;
  } else {
    1;
    // TODO: Integrate charWidth / wcwidth
  };

let getCharacterPositionAndWidth =
    (
      ~indentation: IndentationSettings.t,
      ~viewOffset: int=0,
      line: BufferLine.t,
      i,
    ) => {
  // TODO: Remove this, carried implicitly with BufferLine
  ignore(indentation);
  let (totalOffset, width) = BufferLine.getPositionAndWidth(~index=i, line);

  let actualOffset =
    if (viewOffset > 0) {
      totalOffset - viewOffset;
    } else {
      totalOffset;
    };

  (actualOffset, width);
};

let colorEqual = (c1: Color.t, c2: Color.t) => {
  Float.equal(c1.r, c2.r)
  && Float.equal(c1.g, c2.g)
  && Float.equal(c1.b, c2.b)
  && Float.equal(c1.a, c2.a);
};

let tokenize =
    (~startIndex=0, ~endIndex, line, indentationSettings, colorizer) => {
  let split = (i0, c0, i1, c1) => {
    let (bg1, fg1) = colorizer(i0);
    let (bg2, fg2) = colorizer(i1);

    !colorEqual(bg1, bg2)
    || !colorEqual(fg1, fg2)
    || _isWhitespace(c0) != _isWhitespace(c1)
    /* Always split on tabs */
    || UChar.eq(c0, tab)
    || UChar.eq(c1, tab);
  };

  Tokenizer.tokenize(
    ~startIndex,
    ~endIndex,
    ~f=split,
    ~measure=measure(indentationSettings),
    line,
  )
  |> List.filter(filterRuns)
  |> List.map(textRunToToken(colorizer));
};
