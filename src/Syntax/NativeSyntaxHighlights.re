/*
 NativeSyntaxHighlighting.re
 */

open EditorCoreTypes;
module Core = Oni_Core;
module ThemeToken = Core.ThemeToken;

module type SyntaxHighlighter = {
  type t;

  let hasPendingWork: t => bool;
  let doWork: t => t;
  let updateVisibleRanges: (list(Range.t), t) => t;
  let updateTheme: (TokenTheme.t, t) => t;

  let update:
    (~bufferUpdate: Core.BufferUpdate.t, ~lines: array(string), t) => t;

  let getTokenColors: (t, int) => list(ThemeToken.t);

  // Get a list of lines that have been updated since last clear
  let getUpdatedLines: t => list(int);
  let clearUpdatedLines: t => t;
};

type highlighter('a) = (module SyntaxHighlighter with type t = 'a);

type t =
  | Highlighter({
      highlighter: highlighter('a),
      state: 'a,
    })
    : t;

let _hasTreeSitterScope = (useTreeSitter, scope: string) =>
  if (!useTreeSitter) {
    false;
  } else {
    switch (scope) {
    | "source.json" => true
    /*  | "source.c" => true
        | "source.cpp" => true */
    | _ => false
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
      ~useTreeSitter,
      ~scope,
      ~theme,
      ~getTreesitterScope,
      ~getTextmateGrammar,
      lines: array(string),
    ) => {
  let maybeScopeConverter = getTreesitterScope(scope);

  let allowTreeSitter = _hasTreeSitterScope(useTreeSitter, scope);

  switch (maybeScopeConverter) {
  | Some(scopeConverter) when allowTreeSitter =>
    let ts =
      TreeSitterSyntaxHighlights.create(~theme, ~scopeConverter, lines);
    Highlighter({
      highlighter: (module TreeSitterSyntaxHighlights),
      state: ts,
    });
  | _ =>
    let tm =
      TextMateSyntaxHighlights.create(
        ~scope,
        ~theme,
        ~getTextmateGrammar,
        lines,
      );
    Highlighter({highlighter: (module TextMateSyntaxHighlights), state: tm});
  };
};

let update =
    (~bufferUpdate: Core.BufferUpdate.t, ~lines: array(string), hl: t) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.update(~bufferUpdate, ~lines, state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let getUpdatedLines = (hl: t) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;
  SyntaxHighlighter.getUpdatedLines(state);
};

let clearUpdatedLines = (hl: t) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  let newState = SyntaxHighlighter.clearUpdatedLines(state);
  Highlighter({highlighter: (module SyntaxHighlighter), state: newState});
};

let getTokensForLine = (hl: t, line: int) => {
  let Highlighter({highlighter: (module SyntaxHighlighter), state}) = hl;

  SyntaxHighlighter.getTokenColors(state, line);
};
