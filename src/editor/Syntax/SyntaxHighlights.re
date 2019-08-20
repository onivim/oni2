/*
 SyntaxHighlights.re
 */

open Revery;
open Oni_Core.Types;

module type SyntaxHighlight = {

  type t;

  let empty: t;

  let create: (array(string)) => t;

  let tick: t => t;

  let getTokenColors: (int, int, int) => list(ColorizedToken2.t);

  let update: (BufferUpdate.t, array(string), t) => t;
};

module TestSyntaxHighlight: SyntaxHighlight = {

  type t = unit;

  let empty = ();
  
  let create= (_) => ();

  let tick = () => ();

  let getTokenColors = (_, _, _) => [
    ColorizedToken2.create(~index=0,
                      ~backgroundColor=Colors.red, 
                      ~foregroundColor=Colors.white, ()),
    ColorizedToken2.create(~index=10,
                      ~backgroundColor=Colors.black,
                      ~foregroundColor=Colors.white, ())
  ];

  let update = (_, _, _) => ();
};


module SyntaxHighlights = {
  type t = {
    testSyntaxHighlights: TestSyntaxHighlight.t,
  };

  let empty = {
    testSyntaxHighlights: TestSyntaxHighlight.empty,
  };
};
