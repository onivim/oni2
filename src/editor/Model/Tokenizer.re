/*
 * Tokenizer.re
 */

open Revery;

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

type t = {
  text: string,
  startPosition: Index.t,
  endPosition: Index.t,
  color: Color.t,
};

let _isWhitespace = c => {
  c == ' ' || c == '\t' || c == '\r' || c == '\n';
};

let _isNonWhitespace = c => !_isWhitespace(c);

let _getCharacterVisualWidth = (c) => {
    switch (c) {
    | '\t' => 4
    | _ => 1
    }
};

let _moveToNextMatchingToken = (f, str, startIdx, startOffset) => {
  let idx = ref(startIdx);
  let width = ref(startOffset);
  let length = Zed_utf8.length(str);
  let found = ref(false);

  while (idx^ < length && ! found^) {
    let c = str.[idx^];
    let cWidth = _getCharacterVisualWidth(c);
    found := f(c);

    if (! found^) {
      width := width^ + cWidth;
      idx := idx^ + 1;
    };
  };

  (idx^, width^);
};

let _moveToNextWhitespace = _moveToNextMatchingToken(_isWhitespace);
let _moveToNextNonWhitespace = _moveToNextMatchingToken(_isNonWhitespace);

let _getAllTokens = (s: string, color: Color.t, startPos, endPos, startOffset) => {
  let idx = ref(startPos);
  let lastOffset = ref(startOffset);
  let tokens: ref(list(t)) = ref([]);

  let endPos = min(endPos, Zed_utf8.length(s));

  while (idx^ < endPos) {
    let (startToken, newOffset) = _moveToNextNonWhitespace(s, idx^, startOffset);
    let (endToken, endOffset) = _moveToNextWhitespace(s, startToken + 1, newOffset + 1);


    let endToken = min(endPos, endToken);

    if (startToken < endPos) {
      let length = endToken - startToken;
      let text = Zed_utf8.sub(s, startToken, length);

      let token: t = {
        text,
        startPosition: ZeroBasedIndex(newOffset),
        endPosition: ZeroBasedIndex(endOffset),
        color,
      };

      tokens := List.append([token], tokens^);
    };

    idx := endToken + 1;
    lastOffset := newOffset;
  };
  (tokens^, lastOffset^);
};

let rec getTokens =
        (
          ~indentation,
          tokens: list(ColorizedToken.t),
          pos: int,
          s: string,
          theme: Theme.t,
          colorMap: ColorMap.t,
          offset: int,
        ) => {
            ignore(indentation);
  let defaultForegroundColor: Color.t = theme.colors.editorForeground;
  let defaultBackgroundColor: Color.t = theme.colors.editorBackground;

  let ret: list(t) =
    switch (tokens) {
    | [] => 
        let (tokens, _) = _getAllTokens(s, defaultForegroundColor, pos, Zed_utf8.length(s), 0);
        tokens;
    | [last] =>
      let (tokens, _) = _getAllTokens(
        s,
        ColorMap.get(
          colorMap,
          last.foregroundColor,
          defaultForegroundColor,
          defaultBackgroundColor,
        ),
        last.index,
        Zed_utf8.length(s),
        0,
      )
      tokens;
    | [v1, v2, ...tail] =>
      let (nextBatch, nextOffset) =
        _getAllTokens(
          s,
          ColorMap.get(
            colorMap,
            v1.foregroundColor,
            defaultForegroundColor,
            defaultBackgroundColor,
          ),
          v1.index,
          v2.index,
          offset,
        );
      List.append(
        getTokens(~indentation, [v2, ...tail], v2.index, s, theme, colorMap, nextOffset),
        nextBatch,
      );
    };

  ret;
};

let tokenize = (~indentation=IndentationSettings.default, s, theme, tokenColors, colorMap) => {
    getTokens(~indentation, tokenColors, 0, s, theme, colorMap, 0) |> List.rev;
  };
