/*
 TreeSitterSyntaxHighlighting.re
 */

open Oni_Core;
open Oni_Core.Types;
open Treesitter;
open TreeSitterScopes;

type treeSitterScopeMapperFactory =
  unit => TreeSitterScopes.TextMateConverter.t;

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

let create = (~theme, ~getTreeSitterScopeMapper, lines: array(string)) => {
  let parser = Parser.json();
  let (tree, baseline) = ArrayParser.parse(parser, None, lines);

  let scopeConverter = getTreeSitterScopeMapper();

  let job =
    TreeSitterTokenizerJob.create({tree, lines, theme, scopeConverter});

  {parser, tree, lastBaseline: baseline, lastLines: lines, job};
};

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

let update = (~bufferUpdate: BufferUpdate.t, ~lines: array(string), v: t) => {
  let {parser, lastBaseline, _} = v;
  let delta =
    TreeSitter.ArrayParser.Delta.create(
      lastBaseline,
      Index.toInt0(bufferUpdate.startLine),
      Index.toInt0(bufferUpdate.endLine),
      bufferUpdate.lines,
    );

  let (tree, newBaseline) =
    Log.perf("TreeSitter::parse", () =>
      TreeSitter.ArrayParser.parse(parser, Some(delta), lines)
    );

  let ranges =
    List.init(500, i => i)
    |> List.map(i =>
         Range.ofInt0(
           ~startLine=i,
           ~startCharacter=0,
           ~endLine=i,
           ~endCharacter=100,
           (),
         )
       );

  let job =
    v.job
    |> TreeSitterTokenizerJob.notifyBufferUpdate(bufferUpdate.version)
    |> BufferLineJob.updateContext({
         ...BufferLineJob.getContext(v.job),
         tree,
         lines,
       })
    |> BufferLineJob.setVisibleRanges([ranges]);

  let ret: t = {
    ...v,
    parser,
    tree,
    lastBaseline: newBaseline,
    lastLines: lines,
    job,
  };
  ret;
};
