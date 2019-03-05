/*
 * Tokenizer.re
 */

open Revery;

open Types;

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
  let length = String.length(str);
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

let rec getCurrentTokenColor =
        (
          tokens: list(TextmateClient.ColorizedToken.t),
          startPos: int,
          endPos: int,
        ) => {
  switch (tokens) {
  | [] => [TextmateClient.ColorizedToken.default]
  | [last] => [last]
  | [v1, v2, ...tail] when v1.index <= startPos && v2.index > startPos => [
      v1,
      v2,
      ...tail,
    ]
  | [_, ...tail] => getCurrentTokenColor(tail, startPos, endPos)
  };
};

let tokenize:
  (string, Theme.t, list(TextmateClient.ColorizedToken.t), ColorMap.t) =>
  list(t) =
  (s, theme, tokenColors, colorMap) => {
    let idx = ref(0);
    let length = String.length(s);
    let tokens: ref(list(t)) = ref([]);
    let tokenColorCursor = ref(tokenColors);

    let defaultForegroundColor: Color.t = theme.colors.editorForeground;
    let defaultBackgroundColor: Color.t = theme.colors.editorBackground;

    while (idx^ < length) {
      let startToken = _moveToNextNonWhitespace(s, idx^);
      let endToken = min(length, _moveToNextWhitespace(s, startToken + 1));

      if (startToken < length) {
        let length = endToken - startToken;
        let tokenText = String.sub(s, startToken, length);

        tokenColorCursor :=
          getCurrentTokenColor(tokenColorCursor^, startToken, endToken);
        let color: TextmateClient.ColorizedToken.t =
          List.hd(tokenColorCursor^);
        let foregroundColor =
          ColorMap.get(
            colorMap,
            color.foregroundColor,
            defaultForegroundColor,
            defaultBackgroundColor,
          );

        let token: t = {
          text: tokenText,
          startPosition: ZeroBasedIndex(startToken),
          endPosition: ZeroBasedIndex(endToken),
          color: foregroundColor,
        };

        tokens := List.append([token], tokens^);
      };

      idx := endToken + 1;
    };

    List.rev(tokens^);
  };
