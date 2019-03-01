/*
 * SyntaxHighlighting.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */

module SyntaxHighlight = {
  type t = {
    startIndex: int,
    foregroundColor: int,
    backgroundColor: int,
  };
};

module BufferLineSyntaxHighlights = {
  type t = list(SyntaxHighlight.t);
};

module BufferSyntaxHighlights = {
  type t = {lineToHighlights: IntMap.t(BufferLineSyntaxHighlights.t)};
};

type t = {
    colorMap: ColorMap.t,
    idToBufferSyntaxHighlights: IntMap.t(BufferSyntaxHighlights.t)
};

let create: unit => t = () => {
    colorMap: ColorMap.create(),
    idToBufferSyntaxHighlights: IntMap.empty
};

let reduce: (t, Actions.t) => t =
  (state, action) => {
    switch (action) {
    | _ => state
    };
  };
