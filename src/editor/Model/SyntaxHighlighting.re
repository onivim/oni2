/*
 * SyntaxHighlighting.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;
open Oni_Extensions.TextmateClient;
open Oni_Extensions.TextmateClient.TokenizationResult;

module BufferLineSyntaxHighlights = {
  type t = {
    tokens: list(ColorizedToken.t),
    version: int,
  };

  let create = (~tokens, ~version, ()) => {tokens, version};
};

module BufferSyntaxHighlights = {
  type t = {lineToHighlights: IntMap.t(BufferLineSyntaxHighlights.t)};

  let create = () => {lineToHighlights: IntMap.empty};

  /*
   * Shift adjust the locations of the syntax highlight tokens by a specified _delta_
   * This is important when we add or remove a line, and we're waiting on the textmate
   * highlight service - there'll be a jarring flash where previous highlights are applied.
   */
  let shift = (map: t, startPos: int, endPos: int, delta: int) =>
    if (endPos - startPos == delta) {
      map;
    } else {
      let result =
        IntMap.fold(
          (key, v, prev) =>
            if (delta > 0) {
              if (key < startPos) {
                IntMap.update(key, _opt => Some(v), prev);
              } else {
                IntMap.update(key + delta, _opt => Some(v), prev);
              };
            } else if (key <= endPos) {
              IntMap.update(key, _opt => Some(v), prev);
            } else {
              IntMap.update(key + delta, _opt => Some(v), prev);
            },
          map.lineToHighlights,
          IntMap.empty,
        );

      let ret: t = {lineToHighlights: result};
      ret;
    };

  let getTokensForLine = (v: t, lineId: int) =>
    switch (IntMap.find_opt(lineId, v.lineToHighlights)) {
    | Some(v) => v.tokens
    | None => []
    };

  let update =
      (
        v: t,
        version: int,
        tokens: list(TokenizationResult.tokenizationLineResult),
      ) => {
    let lineToHighlights =
      List.fold_left(
        (curr, item: TokenizationResult.tokenizationLineResult) =>
          IntMap.update(
            item.line,
            prev =>
              switch (prev) {
              | None =>
                Some(
                  BufferLineSyntaxHighlights.create(
                    ~version,
                    ~tokens=item.tokens,
                    (),
                  ),
                )
              | Some(v) =>
                if (v.version < version) {
                  Some(
                    BufferLineSyntaxHighlights.create(
                      ~version,
                      ~tokens=item.tokens,
                      (),
                    ),
                  );
                } else {
                  Some(v);
                }
              },
            curr,
          ),
        v.lineToHighlights,
        tokens,
      );

    let ret: t = {lineToHighlights: lineToHighlights};
    ret;
  };
};

type t = {
  colorMap: ColorMap.t,
  idToBufferSyntaxHighlights: IntMap.t(BufferSyntaxHighlights.t),
};

let getTokensForLine = (v: t, bufferId: int, lineId: int) =>
  switch (IntMap.find_opt(bufferId, v.idToBufferSyntaxHighlights)) {
  | None => []
  | Some(bufferMap) =>
    BufferSyntaxHighlights.getTokensForLine(bufferMap, lineId)
  };

let create: unit => t =
  () => {
    colorMap: ColorMap.create(),
    idToBufferSyntaxHighlights: IntMap.empty,
  };

let reduce: (t, Actions.t) => t =
  (state, action) =>
    switch (action) {
    | SyntaxHighlightColorMap(colorMap) => {...state, colorMap}
    | SyntaxHighlightClear(bufferId) => {
        ...state,
        idToBufferSyntaxHighlights:
          IntMap.update(
            bufferId,
            buffer =>
              switch (buffer) {
              | None => None
              | Some(_) => None
              },
            state.idToBufferSyntaxHighlights,
          ),
      }
    | SyntaxHighlightTokens(tokens) => {
        ...state,
        idToBufferSyntaxHighlights:
          IntMap.update(
            tokens.bufferId,
            buffer =>
              switch (buffer) {
              | None =>
                Some(
                  BufferSyntaxHighlights.update(
                    BufferSyntaxHighlights.create(),
                    tokens.version,
                    tokens.lines,
                  ),
                )
              | Some(v) =>
                Some(
                  BufferSyntaxHighlights.update(
                    v,
                    tokens.version,
                    tokens.lines,
                  ),
                )
              },
            state.idToBufferSyntaxHighlights,
          ),
      }
    | BufferUpdate(bu) => {
        ...state,
        idToBufferSyntaxHighlights:
          IntMap.update(
            bu.id,
            buffer =>
              switch (buffer) {
              | None => None
              | Some(v) =>
                let startLine = bu.startLine |> Index.toZeroBasedInt;
                let endLine = bu.endLine |> Index.toZeroBasedInt;
                Some(
                  BufferSyntaxHighlights.shift(
                    v,
                    startLine,
                    endLine,
                    Array.length(bu.lines) - (endLine - startLine),
                  ),
                );
              },
            state.idToBufferSyntaxHighlights,
          ),
      }
    | _ => state
    };
