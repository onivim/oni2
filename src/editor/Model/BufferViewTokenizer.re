/*
 * BufferViewTokenizer.re
 */

open Revery;

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

open CamomileLibrary;

type t = {
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

let tokenize:
  (string, Theme.t, list(ColorizedToken.t), ColorMap.t) => list(t) =
  (s, theme, tokenColors, colorMap) => {
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

    let measure = _ => 1;
    let split = (i0, c0, i1, c1) => {
      let colorizedToken1 = tokenColorArray[i0];
      let colorizedToken2 = tokenColorArray[i1];
      _isWhitespace(c0) != _isWhitespace(c1)
      || colorizedToken1 !== colorizedToken2;
    };

    let _filterRuns = (r: Tokenizer.TextRun.t) => {
      let len = Zed_utf8.length(r.text);

      if (len == 0) {
        false;
      } else if (_isWhitespace(Zed_utf8.get(r.text, 0))) {
        false;
      } else {
        true;
      };
    };

    let toToken = (r: Tokenizer.TextRun.t) => {
      let startIndex = Index.toZeroBasedInt(r.startIndex);
      let colorIndex = tokenColorArray[startIndex];
      let color =
        ColorMap.get(
          colorMap,
          colorIndex.foregroundColor,
          theme.colors.editorForeground,
          theme.colors.editorBackground,
        );

      let ret: t = {
        text: r.text,
        startPosition: r.startPosition,
        endPosition: r.endPosition,
        color,
      };
      ret;
    };

    Tokenizer.tokenize(~f=split, ~measure, s)
    |> List.filter(_filterRuns)
    |> List.map(toToken);
  };
