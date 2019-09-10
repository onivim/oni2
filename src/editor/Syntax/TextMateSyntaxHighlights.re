/*
 TextMateSyntaxHighlights.re
 */

open Oni_Core;
open Oni_Core.Types;

open Textmate;

type t = TextmateTokenizerJob.t;

let hasPendingWork = (v: t) => !Job.isComplete(v);

let doWork = (v: t) => Job.tick(v);

let updateVisibleRanges = (_ranges, v) => v;

let create = (~scope, ~theme, ~getTextmateGrammar, lines) => {
  let grammar = getTextmateGrammar(scope);
  TextmateTokenizerJob.create(
    ~scope,
    ~theme,
    ~grammar,
    lines,
  );
};

let update = (~bufferUpdate, ~lines, v: t) => {
  ignore(bufferUpdate);
  ignore(lines);
  v;
};

let getTokenColors = (v: t, line: int) => {
  TextmateTokenizerJob.getTokenColors(line, v);
};
