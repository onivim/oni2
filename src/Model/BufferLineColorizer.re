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
      length: int,
      theme: Theme.t,
      tokenColors: list(ColorizedToken.t),
      selection: option(Range.t),
      defaultBackgroundColor: Color.t,
      selectionColor: Color.t,
      matchingPair: option(int),
      searchHighlightRanges: list(Range.t),
    ) => {
  let defaultToken2 =
    ColorizedToken.create(
      ~index=0,
      ~backgroundColor=defaultBackgroundColor,
      ~foregroundColor=theme.editorForeground,
      (),
    );
  let tokenColorArray: array(ColorizedToken.t) =
    Array.make(length, defaultToken2);

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

  let (selectionStart, selectionEnd) =
    switch (selection) {
    | Some(range) =>
      let start = Index.toZeroBased(range.start.column);
      let stop = Index.toZeroBased(range.stop.column);
      start < stop ? (start, stop) : (stop, start);
    | None => ((-1), (-1))
    };

  let tokenColors = List.rev(tokenColors);

  f(tokenColors, length - 1);

  i => {
    let colorIndex = tokenColorArray[i];

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
      List.exists(doesSearchIntersect, searchHighlightRanges);

    let backgroundColor =
      isSearchHighlight ? theme.editorFindMatchBackground : backgroundColor;

    let color = colorIndex.foregroundColor;
    (backgroundColor, color);
  };
};
