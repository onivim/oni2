/*
 * SyntaxHighlighting.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open TextmateClient;
open TextmateClient.TokenizationResult;

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

let create: unit => t =
  () => {
    colorMap: ColorMap.create(),
    idToBufferSyntaxHighlights: IntMap.empty,
  };

let reduce: (t, Actions.t) => t =
  (state, action) => {
    switch (action) {
    | SyntaxHighlightColorMap(colorMap) => {...state, colorMap}
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
    | _ => state
    };
  };
