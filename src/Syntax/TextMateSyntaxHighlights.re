/*
 TextMateSyntaxHighlights.re
 */

open Oni_Core;

type t = TextmateTokenizerJob.t;

module Log = (val Log.withNamespace("Oni2.TextMateSyntaxHighlights"));

let hasPendingWork = (v: t) => !Job.isComplete(v);

let doWork = (v: t) => {
  Log.info("Starting job...");
  let hasRun = Job.getPendingWork(v).hasRun;
  // For first render of a buffer, spend a little extra time on the tokenization
  // so that can minimize flicker.
  let result =
    if (!hasRun) {
      Job.tick(~budget=Some(0.025), v);
    } else {
      Job.tick(v);
    };
  Log.info("Finished job.");
  result;
};

let updateTheme = (theme, v) => TextmateTokenizerJob.onTheme(theme, v);

let updateVisibleRanges = (_ranges, v) => v;

let create = (~scope, ~theme, ~getTextmateGrammar, lines) => {
  Log.info("Creating highlighter for scope: " ++ scope);
  let grammarRepository =
    Textmate.GrammarRepository.create(scope => getTextmateGrammar(scope));
  Log.info("- Created grammar repository.");
  let ret =
    TextmateTokenizerJob.create(~scope, ~theme, ~grammarRepository, lines);
  Log.info("Finished creating highligher for scope: " ++ scope);
  ret;
};

let update = (~bufferUpdate, ~lines, v: t) => {
  TextmateTokenizerJob.onBufferUpdate(bufferUpdate, lines, v);
};

let getTokenColors = (v: t, line: int) => {
  TextmateTokenizerJob.getTokenColors(line, v);
};

let getUpdatedLines = (tm: t) => {
  Job.getCompletedWork(tm).latestLines;
};

let clearUpdatedLines = (tm: t) =>
  TextmateTokenizerJob.clearUpdatedLines(tm);

let getDiscoveredKeywords = (_tm: t) => ["Keyword", "Discovery"];

let clearDiscoveredKeywords = tm => tm;
