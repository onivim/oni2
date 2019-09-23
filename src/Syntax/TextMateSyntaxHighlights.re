/*
 TextMateSyntaxHighlights.re
 */

open Oni_Core;

type t = TextmateTokenizerJob.t;

let hasPendingWork = (v: t) => !Job.isComplete(v);

let doWork = (v: t) => {
  let hasRun = Job.getPendingWork(v).hasRun;
  // For first render of a buffer, spend a little extra time on the tokenization
  // so that can minimize flicker.
  if (!hasRun) {
    Job.tick(~budget=Some(0.025), v);
  } else {
    Job.tick(v);
  };
};

let updateTheme = (theme, v) => TextmateTokenizerJob.onTheme(theme, v);

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
