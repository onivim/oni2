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
type t = ByteIndex.t => themedToken;

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
};

let create =
    (
      ~startByte: ByteIndex.t,
      ~defaultBackgroundColor: Color.t,
      ~defaultForegroundColor: Color.t,
      ~selectionHighlights: option(ByteRange.t),
      ~selectionColor: Color.t,
      ~matchingPair: option(ByteIndex.t),
      ~searchHighlights: list(ByteRange.t),
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

  let matchingPair = matchingPair |> Option.map(ByteIndex.toInt);

  let startByteIdx = ByteIndex.toInt(startByte);
  let (defaultToken, tokens) =
    Internal.getFirstRelevantToken(
      ~default=initialDefaultToken,
      ~startByte=startByteIdx,
      themedTokens,
    );

  let (selectionStart, selectionEnd) =
    switch (selectionHighlights) {
    | Some(range) =>
      let start = ByteIndex.toInt(range.start.byte);
      let stop = ByteIndex.toInt(range.stop.byte);
      start < stop ? (start, stop) : (stop, start);
    | None => ((-1), (-1))
    };

  (byteIndex: ByteIndex.t) => {
    let i = ByteIndex.toInt(byteIndex);
    let colorIndex =
      Feature_Syntax.Tokens.getAt(~byteIndex, tokens)
      |> Option.value(~default=defaultToken);

    let matchingPair =
      switch (matchingPair) {
      | None => (-1)
      | Some(v) => v
      };

    let backgroundColor =
      i >= selectionStart && i < selectionEnd || i == matchingPair
        ? selectionColor : defaultBackgroundColor;

    let doesSearchIntersect = (range: ByteRange.t) => {
      ByteIndex.toInt(range.start.byte) <= i
      && ByteIndex.toInt(range.stop.byte) > i;
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
