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

module Internal = {
  let getFirstRelevantToken = (~default, ~startByte, tokens) => {
    let rec loop = (default, tokens: list(ThemeToken.t)) => {
      switch (tokens) {
      | [] => (default, tokens)
      | [_] => (default, tokens)
      | [token, nextToken, ...tail] =>
        if (token.index >= startByte) {
          (default, tokens);
        } else if (token.index < startByte && nextToken.index >= startByte) {
          (token, [nextToken, ...tail]);
        } else {
          loop(token, [nextToken, ...tail]);
        }
      };
    };

    loop(default, tokens);
  };

  let getTokenAtByte = (~default, ~byteIndex, tokens) => {
    let rec loop = (lastToken, tokens: list(ThemeToken.t)) => {
      switch (tokens) {
      | [] => lastToken
      | [token] => token.index <= byteIndex ? token : lastToken
      | [token, ...tail] =>
        if (token.index > byteIndex) {
          lastToken;
        } else {
          loop(token, tail);
        }
      };
    };

    loop(default, tokens);
  };
};

let create =
    (
      ~startByte,
      ~defaultBackgroundColor: Color.t,
      ~defaultForegroundColor: Color.t,
      ~selectionHighlights: option(Range.t),
      ~selectionColor: Color.t,
      ~matchingPair: option(int),
      ~searchHighlights: list(Range.t),
      ~searchHighlightColor: Color.t,
      themedTokens: list(ThemeToken.t),
    ) => {
  let initialDefaultToken =
    ThemeToken.create(
      ~index=0,
      ~backgroundColor=defaultBackgroundColor,
      ~foregroundColor=defaultForegroundColor,
      ~syntaxScope=SyntaxScope.none,
      (),
    );

  let (defaultToken, tokens) =
    Internal.getFirstRelevantToken(
      ~default=initialDefaultToken,
      ~startByte,
      themedTokens,
    );

  let (selectionStart, selectionEnd) =
    switch (selectionHighlights) {
    | Some(range) =>
      let start = Index.toZeroBased(range.start.column);
      let stop = Index.toZeroBased(range.stop.column);
      start < stop ? (start, stop) : (stop, start);
    | None => ((-1), (-1))
    };

  i => {
    let colorIndex =
      Internal.getTokenAtByte(~byteIndex=i, ~default=defaultToken, tokens);

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
