/*
 * BufferLineColorizer.re
 */

open EditorCoreTypes;
open Revery;

open Oni_Core;

type tokenColor = (Color.t, Color.t);
type t = int => tokenColor;

let create =
    (
      ~startIndex,
      ~endIndex,
      ~defaultBackgroundColor: Color.t,
      ~defaultForegroundColor: Color.t,
      ~selectionHighlights: option(Range.t),
      ~selectionColor: Color.t,
      ~matchingPair: option(int),
      ~searchHighlights: list(Range.t),
      ~searchHighlightColor: Color.t,
      tokenColors: list(ColorizedToken.t),
    ) => {
  let defaultToken2 =
    ColorizedToken.create(
      ~index=0,
      ~backgroundColor=defaultBackgroundColor,
      ~foregroundColor=defaultForegroundColor,
      ~syntaxScope=SyntaxScope.None,
      (),
    );

  let length = max(endIndex - startIndex, 1);

  let tokenColorArray: array(ColorizedToken.t) =
    Array.make(length, defaultToken2);

  let rec f = (tokens: list(ColorizedToken.t), start) =>
    switch (tokens) {
    | [] => ()
    | [hd, ...tail] =>
      let adjIndex = hd.index - startIndex;

      let pos = ref(start);

      while (pos^ >= adjIndex && pos^ >= 0 && pos^ < length) {
        tokenColorArray[pos^] = hd;
        decr(pos);
      };
      if (hd.index < startIndex) {
        ();
      } else {
        f(tail, pos^);
      };
    };

  let (selectionStart, selectionEnd) =
    switch (selectionHighlights) {
    | Some(range) =>
      let start = Index.toZeroBased(range.start.column);
      let stop = Index.toZeroBased(range.stop.column);
      start < stop ? (start, stop) : (stop, start);
    | None => ((-1), (-1))
    };

  let tokenColors = List.rev(tokenColors);

  f(tokenColors, length - 1);

  i => {
    let colorIndex =
      if (i < startIndex) {
        tokenColorArray[0];
      } else if (i >= endIndex) {
        tokenColorArray[Array.length(tokenColorArray) - 1];
      } else {
        tokenColorArray[i - startIndex];
      };

    let matchingPair =
      switch (matchingPair) {
      | None => (-1)
      | Some(v) => v
      };

    let backgroundColor =
      i >= selectionStart && i < selectionEnd || i == matchingPair
        ? selectionColor : defaultBackgroundColor;

    let doesSearchIntersect = (range: Range.t) => {
      Index.toZeroBased(range.start.column) <= i
      && Index.toZeroBased(range.stop.column) > i;
    };

    let isSearchHighlight =
      List.exists(doesSearchIntersect, searchHighlights);

    let backgroundColor =
      isSearchHighlight ? searchHighlightColor : backgroundColor;

    let color = colorIndex.foregroundColor;
    (backgroundColor, color);
  };
};
