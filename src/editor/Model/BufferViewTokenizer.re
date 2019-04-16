/*
 * BufferViewTokenizer.re
 */

open Revery;

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

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

let textRunToToken =
    (
      colorMap,
      theme: Theme.t,
      tokenColorArray: array(ColorizedToken.t),
      r: Tokenizer.TextRun.t,
    ) => {
  let startIndex = Index.toZeroBasedInt(r.startIndex);
  let colorIndex = tokenColorArray[startIndex];

  let firstChar = Zed_utf8.get(r.text, 0);

  let tokenType =
    if (UChar.eq(firstChar, tab)) {
      Tab;
    } else if (UChar.eq(firstChar, space)) {
      Whitespace;
    } else {
      Text;
    };

  let color =
    ColorMap.get(
      colorMap,
      colorIndex.foregroundColor,
      theme.colors.editorForeground,
      theme.colors.editorBackground,
    );

  let ret: t = {
    tokenType,
    text: r.text,
    startPosition: r.startPosition,
    endPosition: r.endPosition,
    color,
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

let tokenize:
  (
    string,
    Theme.t,
    list(ColorizedToken.t),
    ColorMap.t,
    IndentationSettings.t
  ) =>
  list(t) =
  (s, theme, tokenColors, colorMap, indentationSettings) => {
    let len = Zed_utf8.length(s);
    let tokenColorArray: array(ColorizedToken.t) =
      Array.make(len, ColorizedToken.default);

    let rec f = (tokens: list(ColorizedToken.t), start) =>
      switch (tokens) {
      | [] => ()
      | [hd, ...tail] =>
        let pos = ref(start);
        while (pos^ >= hd.index) {
          tokenColorArray[pos^] = hd;
          decr(pos);
        };
        f(tail, pos^);
      };

    let tokenColors = List.rev(tokenColors);

    f(tokenColors, len - 1);

    let split = (i0, c0, i1, c1) => {
      let colorizedToken1 = tokenColorArray[i0];
      let colorizedToken2 = tokenColorArray[i1];
      _isWhitespace(c0) != _isWhitespace(c1)
      || colorizedToken1 !== colorizedToken2
      /* Always split on tabs */
      || UChar.eq(c0, tab)
      || UChar.eq(c1, tab);
    };

    Tokenizer.tokenize(~f=split, ~measure=measure(indentationSettings), s)
    |> List.filter(filterRuns)
    |> List.map(textRunToToken(colorMap, theme, tokenColorArray));
  };
