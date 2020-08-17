/*
     ArrayParser.rei

     Specialized parser for arrays of string
 */

type t;

module Baseline: {
  /*
   [Baseline.t] contains a baseline parse result, to be used
   to create an incremental delta.
   */
  type t;
};

module Delta: {
  /* [Delta.t] describes updates to a Tree for incremental parsing */
  type t;

  /*
     [create(baseline, oldStartLine, oldEndLine, updates)] creates an incremental
     update to speed up the [parse].
   */

  let create: (Baseline.t, int, int, array(string)) => t;
};

/*
   [parse(parser, delta, contents] parses a document, provided by a string array [contents],
   and returns a tuple of a `Tree.t` and `Baseline.t.`

   The `baseline` can be used to create a delta, and optionally provided for incremental parsing.
 */
let parse:
  (Parser.t, option(Delta.t), array(string)) => (Tree.t, Baseline.t);
