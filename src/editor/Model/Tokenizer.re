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

let _moveToNextMatchingToken = (f, str, startIdx) => {
  let idx = ref(startIdx);
  let length = Zed_utf8.length(str);
  let found = ref(false);

  while (idx^ < length && ! found^) {
    let c = str.[idx^];
    found := f(c);

    if (! found^) {
      idx := idx^ + 1;
    };
  };

  idx^;
};

let _moveToNextWhitespace = _moveToNextMatchingToken(_isWhitespace);
let _moveToNextNonWhitespace = _moveToNextMatchingToken(_isNonWhitespace);

let _getAllTokens = (s: string, color: Color.t, startPos, endPos) => {
  let idx = ref(startPos);
  let tokens: ref(list(t)) = ref([]);

  let endPos = min(endPos, Zed_utf8.length(s));

  while (idx^ < endPos) {
    let startToken = _moveToNextNonWhitespace(s, idx^);
    let endToken = min(endPos, _moveToNextWhitespace(s, startToken + 1));

    if (startToken < endPos) {
      let length = endToken - startToken;
      let text = Zed_utf8.sub(s, startToken, length);

      let token: t = {
        text,
        startPosition: ZeroBasedIndex(startToken),
        endPosition: ZeroBasedIndex(endToken),
        color,
      };

      tokens := List.append([token], tokens^);
    };

    idx := endToken + 1;
  };
  tokens^;
};

let rec getTokens =
        (
          tokens: list(ColorizedToken.t),
          pos: int,
          s: string,
          theme: Theme.t,
          colorMap: ColorMap.t,
        ) => {
  let defaultForegroundColor: Color.t = theme.colors.editorForeground;
  let defaultBackgroundColor: Color.t = theme.colors.editorBackground;

  let ret: list(t) =
    switch (tokens) {
    | [] => _getAllTokens(s, defaultForegroundColor, pos, Zed_utf8.length(s))
    | [last] =>
      _getAllTokens(
        s,
        ColorMap.get(
          colorMap,
          last.foregroundColor,
          defaultForegroundColor,
          defaultBackgroundColor,
        ),
        last.index,
        Zed_utf8.length(s),
      )
    | [v1, v2, ...tail] =>
      let nextBatch =
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
        );
      List.append(
        getTokens([v2, ...tail], v2.index, s, theme, colorMap),
        nextBatch,
      );
    };

  ret;
};

let tokenize:
  (string, Theme.t, list(ColorizedToken.t), ColorMap.t) => list(t) =
  (s, theme, tokenColors, colorMap) => {
    getTokens(tokenColors, 0, s, theme, colorMap) |> List.rev;
  };
