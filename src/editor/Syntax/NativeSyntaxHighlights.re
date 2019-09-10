/*
 NativeSyntaxHighlighting.re
 */

module Core = Oni_Core;

type t =
  | TextMate(TextMateSyntaxHighlights.t)
  | TreeSitter(TreeSitterSyntaxHighlights.t)
  | None;

let default = None;

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

let canHandleScope = (configuration: Core.Configuration.t, scope: string) => {
  let nativeHighlightingEnabled =
    Core.Configuration.getValue(
      c => c.experimentalNativeTextMate,
      configuration,
    );

  if (nativeHighlightingEnabled) {
    true;
  } else {
    _hasTreeSitterScope(configuration, scope);
  };
};

let anyPendingWork = v => {
  switch (v) {
  | None => false
  | TextMate(tm) => TextMateSyntaxHighlights.hasPendingWork(tm)
  | TreeSitter(ts) => TreeSitterSyntaxHighlights.hasPendingWork(ts)
  };
};

let doWork = v => {
  switch (v) {
  | None => v
  | TextMate(tm) => TextMate(TextMateSyntaxHighlights.doWork(tm))
  | TreeSitter(ts) => TreeSitter(TreeSitterSyntaxHighlights.doWork(ts))
  };
};

let updateVisibleRanges = (ranges, v) => {
  switch (v) {
  | None => v
  | TextMate(tm) =>
    TextMate(TextMateSyntaxHighlights.updateVisibleRanges(ranges, tm))
  | TreeSitter(ts) =>
    TreeSitter(TreeSitterSyntaxHighlights.updateVisibleRanges(ranges, ts))
  };
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
      TreeSitter(ts);
    }
    : {
      let tm =
        TextMateSyntaxHighlights.create(
          ~theme,
          ~getTextmateGrammar,
          lines,
        );
      TextMate(tm);
    };
};

let update =
    (~bufferUpdate: Core.Types.BufferUpdate.t, ~lines: array(string), v: t) => {
  switch (v) {
  | TextMate(tm) =>
    TextMate(TextMateSyntaxHighlights.update(~bufferUpdate, ~lines, tm))
  | TreeSitter(ts) =>
    let newTs: TreeSitterSyntaxHighlights.t =
      TreeSitterSyntaxHighlights.update(~bufferUpdate, ~lines, ts);
    TreeSitter(newTs);
  | _ => v
  };
};

let getTokensForLine = (v: t, line: int) => {
  switch (v) {
  | TextMate(tm) => TextMateSyntaxHighlights.getTokenColors(tm, line)
  | TreeSitter(ts) => TreeSitterSyntaxHighlights.getTokenColors(ts, line)
  | _ => []
  };
};
