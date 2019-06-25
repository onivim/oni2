/*
 * BufferLineColorizer.re
 */

open Revery;

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

type tokenColor = (Color.t, Color.t);
type t = int => tokenColor;

let create =
    (
      length: int,
      theme: Theme.t,
      tokenColors: list(ColorizedToken.t),
      colorMap: ColorMap.t,
      selection: option(Range.t),
      defaultBackgroundColor: Color.t,
      selectionColor: Color.t,
      matchingPair: option(int),
    ) => {
  let tokenColorArray: array(ColorizedToken.t) =
    Array.make(length, ColorizedToken.default);

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

    let color =
      ColorMap.get(
        colorMap,
        colorIndex.foregroundColor,
        theme.colors.editorForeground,
        theme.colors.editorBackground,
      );
    (backgroundColor, color);
  };
};
