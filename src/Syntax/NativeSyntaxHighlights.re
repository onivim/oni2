/*
 NativeSyntaxHighlighting.re
 */

module Core = Oni_Core;
module ColorizedToken = Core.Types.ColorizedToken;
module Range = Core.Range;

module type SyntaxHighlighter = {
  type t;

  let hasPendingWork: t => bool;
  let doWork: t => t;
  let updateVisibleRanges: (list(Range.t), t) => t;
  let updateTheme: (TokenTheme.t, t) => t;

  let update:
    (~bufferUpdate: Core.BufferUpdate.t, ~lines: array(string), t) => t;

  let getTokenColors: (t, int) => list(ColorizedToken.t);
};

type highlighter('a) = (module SyntaxHighlighter with type t = 'a);

type t =
  | Highlighter({
      highlighter: highlighter('a),
      state: 'a,
    })
    : t;

let _hasTreeSitterScope = (configuration, scope: string) => {
  let treeSitterEnabled =
    Core.Configuration.getValue(c => c.experimentalTreeSitter, configuration);

  if (!treeSitterEnabled) {
    false;
  } else {
    switch (scope) {
    | "source.json" => true
    /*  | "source.c" => true
        | "source.cpp" => true */
    | _ => false
    };
  };
};

let anyPendingWork = hl => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  SyntaxHighlighter.hasPendingWork(state);
};

let doWork = hl => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.doWork(state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let updateVisibleRanges = (ranges, hl) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.updateVisibleRanges(ranges, state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let updateTheme = (theme, hl) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.updateTheme(theme, state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let create =
    (
      ~configuration,
      ~scope,
      ~theme,
      ~getTextmateGrammar,
      ~getTreeSitterScopeMapper,
      lines: array(string),
    ) => {
  _hasTreeSitterScope(configuration, scope)
    ? {
      let ts =
        TreeSitterSyntaxHighlights.create(
          ~theme,
          ~getTreeSitterScopeMapper,
          lines,
        );
      Highlighter({
        highlighter: (module TreeSitterSyntaxHighlights),
        state: ts,
      });
    }
    : {
      let tm =
        TextMateSyntaxHighlights.create(
          ~scope,
          ~theme,
          ~getTextmateGrammar,
          lines,
        );
      Highlighter({
        highlighter: (module TextMateSyntaxHighlights),
        state: tm,
      });
    };
};

let update =
    (~bufferUpdate: Core.BufferUpdate.t, ~lines: array(string), hl: t) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.update(~bufferUpdate, ~lines, state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let getTokensForLine = (hl: t, line: int) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  SyntaxHighlighter.getTokenColors(state, line);
};
