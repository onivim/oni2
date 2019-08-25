/*
 * BufferLineColorizer.re
 */

open Revery;

open Oni_Core;
open Oni_Extensions;
open Oni_Core.Types;

type tokenColor = (Color.t, Color.t);
type t = int => tokenColor;

let create =
    (
      length: int,
      theme: Theme.t,
      tokenColors: list(ColorizedToken2.t),
      selection: option(Range.t),
      defaultBackgroundColor: Color.t,
      selectionColor: Color.t,
      matchingPair: option(int),
      searchHighlightRanges: list(Range.t),
    ) => {
  let defaultToken2 =
    ColorizedToken2.create(
      ~index=0,
      ~backgroundColor=defaultBackgroundColor,
      ~foregroundColor=theme.editorForeground,
      (),
    );
  let tokenColorArray: array(ColorizedToken2.t) =
    Array.make(length, defaultToken2);

  let rec f = (tokens: list(ColorizedToken2.t), start) =>
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
    | Some(v) =>
      let s = Index.toZeroBasedInt(v.startPosition.character);
      let e = Index.toZeroBasedInt(v.endPosition.character);
      e > s ? (s, e) : (e, s);
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
      Range.(
        Index.toInt0(range.startPosition.character) <= i
        && Index.toInt0(range.endPosition.character) > i
      );
    };

    let isSearchHighlight =
      List.exists(doesSearchIntersect, searchHighlightRanges);

    let backgroundColor =
      isSearchHighlight ? theme.editorFindMatchBackground : backgroundColor;

    let color = colorIndex.foregroundColor;
    (backgroundColor, color);
  };
};
