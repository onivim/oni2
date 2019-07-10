/*
 * BufferViewTokenizer.re
 */

open Revery;

open Oni_Core;
open Oni_Core.Types;

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
  let len = Zed_utf8.length(r.text);

  if (len == 0) {
    false;
  } else {
    true;
  };
};

type colorizer = int => (Color.t, Color.t);

let textRunToToken = (colorizer: colorizer, r: Tokenizer.TextRun.t) => {
  let startIndex = Index.toZeroBasedInt(r.startIndex);
  let (bg, fg) = colorizer(startIndex);

  let firstChar = Zed_utf8.get(r.text, 0);

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

let measure = (indentationSettings: IndentationSettings.t, c) =>
  if (UChar.eq(c, tab)) {
    indentationSettings.tabSize;
  } else {
    1;
  };

let getCharacterPositionAndWidth =
    (~indentation: IndentationSettings.t, str, i) => {
  let x = ref(0);
  let totalOffset = ref(0);
  let len = Zed_utf8.length(str);

  let measure = measure(indentation);

  while (x^ < len && x^ < i) {
    let c = Zed_utf8.get(str, x^);
    let width = measure(c);

    totalOffset := totalOffset^ + width;

    incr(x);
  };

  let width = i < len ? measure(Zed_utf8.get(str, i)) : 1;

  (totalOffset^, width);
};

let colorEqual = (c1: Color.t, c2: Color.t) => {
  Float.equal(c1.r, c2.r)
  && Float.equal(c1.g, c2.g)
  && Float.equal(c1.b, c2.b)
  && Float.equal(c1.a, c2.a);
};

let tokenize =
    (~startIndex=0, ~endIndex=(-1), s, indentationSettings, colorizer) => {
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
    s,
  )
  |> List.filter(filterRuns)
  |> List.map(textRunToToken(colorizer));
};
