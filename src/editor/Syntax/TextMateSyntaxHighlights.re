/*
 TextMateSyntaxHighlights.re
 */

open Oni_Core;

type t = TextmateTokenizerJob.t;

let hasPendingWork = (v: t) => !Job.isComplete(v);

let doWork = (v: t) => Job.tick(v);

let updateVisibleRanges = (_ranges, v) => v;

let create = (~scope, ~theme, ~getTextmateGrammar, lines) => {
  let grammarRepository =
    Textmate.GrammarRepository.create(scope => getTextmateGrammar(scope));
  TextmateTokenizerJob.create(~scope, ~theme, ~grammarRepository, lines);
};

let update = (~bufferUpdate, ~lines, v: t) => {
  TextmateTokenizerJob.onBufferUpdate(bufferUpdate, lines, v);
};

let getTokenColors = (v: t, line: int) => {
  TextmateTokenizerJob.getTokenColors(line, v);
};
