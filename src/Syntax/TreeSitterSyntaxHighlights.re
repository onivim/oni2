/*
 TreeSitterSyntaxHighlighting.re
 */

open EditorCoreTypes;
open Oni_Core;
open Treesitter;

module Log = (
  val Log.withNamespace("Oni2.Syntax.TreeSitterSyntaxHighlights")
);

type t = {
  parser: Parser.t,
  tree: Tree.t,
  lastBaseline: ArrayParser.Baseline.t,
  lastLines: array(string),
  job: TreeSitterTokenizerJob.t,
};
let someOrNone = v =>
  switch (v) {
  | Some(v) => v
  | None => ""
  };

let scopesToStrings = (scopes: list(TreeSitter.Syntax.scope)) => {
  List.map(
    v => {
      let (_, s) = v;
      s;
    },
    scopes,
  );
};

let getParserFromScope = language =>
  switch (language) {
  | "source.json" => Some(Parser.json())
  | "source.c" => Some(Parser.c())
  | "source.cpp" => Some(Parser.cpp())
  | "source.python" => Some(Parser.python())
  | "source.javascript" => Some(Parser.javascript())
  | "source.typescript" => Some(Parser.typescript())
  | "source.tsx" => Some(Parser.tsx())
  | _ => None
  };

let create = (~theme, ~scopeConverter, ~parser, lines: array(string)) => {
  let (tree, baseline) = ArrayParser.parse(parser, None, lines);

  let job =
    TreeSitterTokenizerJob.create({tree, lines, theme, scopeConverter});

  {parser, tree, lastBaseline: baseline, lastLines: lines, job};
};

let updateTheme = (theme, v) => {
  let job = TreeSitterTokenizerJob.updateTheme(theme, v.job);
  {...v, job};
};

let clearUpdatedLines = ts => {
  ...ts,
  job: TreeSitterTokenizerJob.clearUpdatedLines(ts.job),
};

let getUpdatedLines = ts => TreeSitterTokenizerJob.getUpdatedLines(ts.job);

let hasPendingWork = v => !TreeSitterTokenizerJob.isComplete(v.job);
let doWork = v => {
  switch (hasPendingWork(v)) {
  | false => v
  | true => {...v, job: Job.tick(v.job)}
  };
};

let getTokenColors = (v: t, line: int) => {
  TreeSitterTokenizerJob.getTokensForLine(line, v.job);
};

let updateVisibleRanges = (ranges, v: t) => {
  let job = v.job |> BufferLineJob.setVisibleRanges([ranges]);

  {...v, job};
};

let update = (~bufferUpdate: BufferUpdate.t, ~lines: array(string), v: t) => {
  let {parser, lastBaseline, _} = v;
  let delta =
    TreeSitter.ArrayParser.Delta.create(
      lastBaseline,
      Index.toZeroBased(bufferUpdate.startLine),
      Index.toZeroBased(bufferUpdate.endLine),
      bufferUpdate.lines,
    );

  let (tree, newBaseline) =
    Oni_Core.Log.perf("TreeSitter::parse", () =>
      TreeSitter.ArrayParser.parse(parser, Some(delta), lines)
    );

  let job =
    v.job
    |> TreeSitterTokenizerJob.notifyBufferUpdate(bufferUpdate.version)
    |> BufferLineJob.updateContext({
         ...BufferLineJob.getContext(v.job),
         tree,
         lines,
       });

  let ret: t = {
    parser,
    tree,
    lastBaseline: newBaseline,
    lastLines: lines,
    job,
  };
  ret;
};
