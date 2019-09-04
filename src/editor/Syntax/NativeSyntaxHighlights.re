/*
 NativeSyntaxHighlighting.re
 */

module Core = Oni_Core;

type t =
  | TreeSitter(TreeSitterSyntaxHighlights.t)
  | None;

let default = None;

let canHandleScope = (configuration: Core.Configuration.t, scope: string) => {
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

let anyPendingWork = v => {
  switch (v) {
  | None => false
  | TreeSitter(ts) => TreeSitterSyntaxHighlights.hasPendingWork(ts)
  };
};

let doWork = v => {
  switch (v) {
  | None => v
  | TreeSitter(ts) => TreeSitter(TreeSitterSyntaxHighlights.doWork(ts))
  };
};

let updateVisibleRanges = (ranges, v) => {
  switch (v) {
  | None => v
  | TreeSitter(ts) =>
    TreeSitter(TreeSitterSyntaxHighlights.updateVisibleRanges(ranges, ts))
  };
};

let create = (~theme, ~getTreeSitterScopeMapper, lines: array(string)) => {
  let ts =
    TreeSitterSyntaxHighlights.create(
      ~theme,
      ~getTreeSitterScopeMapper,
      lines,
    );
  TreeSitter(ts);
};

let update =
    (~bufferUpdate: Core.Types.BufferUpdate.t, ~lines: array(string), v: t) => {
  switch (v) {
  | TreeSitter(ts) =>
    let newTs: TreeSitterSyntaxHighlights.t =
      TreeSitterSyntaxHighlights.update(~bufferUpdate, ~lines, ts);
    TreeSitter(newTs);
  | _ => v
  };
};

let getTokensForLine = (v: t, line: int) => {
  switch (v) {
  | TreeSitter(ts) => TreeSitterSyntaxHighlights.getTokenColors(ts, line)
  | _ => []
  };
};
