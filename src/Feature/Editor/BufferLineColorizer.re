/*
 * BufferLineColorizer.re
 */

open EditorCoreTypes;
open Revery;

open Oni_Core;

type themedToken = {
  color: Color.t,
  backgroundColor: Color.t,
  italic: bool,
  bold: bool,
};
type t = int => themedToken;

let create =
    (
      ~startByte,
      ~endByte,
      ~defaultBackgroundColor: Color.t,
      ~defaultForegroundColor: Color.t,
      ~selectionHighlights: option(Range.t),
      ~selectionColor: Color.t,
      ~matchingPair: option(int),
      ~searchHighlights: list(Range.t),
      ~searchHighlightColor: Color.t,
      themedTokens: list(ThemeToken.t),
    ) => {
  let defaultToken2 =
    ThemeToken.create(
      ~index=0,
      ~backgroundColor=defaultBackgroundColor,
      ~foregroundColor=defaultForegroundColor,
      ~syntaxScope=SyntaxScope.none,
      (),
    );

  let length = max(endByte - startByte, 1);

  let themedTokenArray: array(ThemeToken.t) =
    Array.make(length, defaultToken2);

  let rec f = (tokens: list(ThemeToken.t), start) =>
    switch (tokens) {
    | [] => ()
    | [hd, ...tail] =>
      let adjIndex = hd.index - startByte;

      let pos = ref(start);

      while (pos^ >= adjIndex && pos^ >= 0 && pos^ < length) {
        themedTokenArray[pos^] = hd;
        decr(pos);
      };
      if (hd.index < startByte) {
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

  let themedTokens = List.rev(themedTokens);

  f(themedTokens, length - 1);

  i => {
    let colorIndex =
      if (i < startByte) {
        themedTokenArray[0];
      } else if (i >= endByte) {
        themedTokenArray[Array.length(themedTokenArray) - 1];
      } else {
        themedTokenArray[i - startByte];
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
    {
      backgroundColor,
      color,
      bold: colorIndex.bold,
      italic: colorIndex.italic,
    };
  };
};
