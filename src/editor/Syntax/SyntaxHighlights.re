/*
 SyntaxHighlights.re
 */

open Revery;
open Oni_Core.Types;

module type SyntaxHighlight = {
  type t;

  let empty: t;

  let hasPendingWork: t => bool;
  let doChunkOfWork: t => t;

  let create: array(string) => t;

  let getTokenColors: (t, int) => list(ColorizedToken2.t);

  let update: (BufferUpdate.t, array(string), t) => t;
};

module TestSyntaxHighlight: SyntaxHighlight = {
  type t = unit;

  let empty = ();

  let create = _ => ();

  let hasPendingWork = () => false;
  let doChunkOfWork = () => ();

  let getTokenColors = (_, _) => [
    ColorizedToken2.create(
      ~index=0,
      ~backgroundColor=Colors.red,
      ~foregroundColor=Colors.green,
      (),
    ),
    ColorizedToken2.create(
      ~index=10,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.red,
      (),
    ),
    ColorizedToken2.create(
      ~index=20,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.blue,
      (),
    ),
  ];

  let update = (_, _, _) => ();
};

module TreeSitterSyntaxHighlight: SyntaxHighlight = {
  type t = unit;

  let empty = ();

  let create = _ => ();

  let hasPendingWork = () => false;
  let doChunkOfWork = () => ();

  let getTokenColors = (_, _) => [
    ColorizedToken2.create(
      ~index=0,
      ~backgroundColor=Colors.red,
      ~foregroundColor=Colors.green,
      (),
    ),
    ColorizedToken2.create(
      ~index=10,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.red,
      (),
    ),
    ColorizedToken2.create(
      ~index=20,
      ~backgroundColor=Colors.black,
      ~foregroundColor=Colors.blue,
      (),
    ),
  ];

  let update = (_, _, _) => ();
};

type syntaxHighlightStrategy =
  | Test
  | TreeSitter
  | ReasonML
  | TextMate
  | None;

module SyntaxHighlights = {

  
  type t = {
    strategy: syntaxHighlightStrategy,
    testSyntaxHighlights: TestSyntaxHighlight.t,
  };

  let default = {
    strategy: None,
    testSyntaxHighlights: TestSyntaxHighlight.empty,
  };

  let create = (strategy: syntaxHighlightStrategy, lines: array(string)) => {
    switch (strategy) {
    | Test => {
      ...default,
      strategy: Test,
      testSyntaxHighlights: TestSyntaxHighlight.create(lines),
    }
    | _ => {
      default
    }
    }
  };

  let empty = {
    strategy: Test,
    testSyntaxHighlights: TestSyntaxHighlight.empty,
  };

  let getTokensForLine = (v: t, line: int) => {
    switch (v.strategy) {
    | Test => TestSyntaxHighlight.getTokenColors(v.testSyntaxHighlights, line);
    | _ => []
    }
  };
};
