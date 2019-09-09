/*
 TextMateSyntaxHighlights.re
 */

open Oni_Core;
open Oni_Core.Types;

type t = unit;


let hasPendingWork = (_v: t) => false;

let doWork = (v: t) => v;

let updateVisibleRanges = (_ranges, v) => v;


let create = (~theme, ~getTextMateGrammar, lines) => {
  ignore(getTextMateGrammar);
  ignore(theme);
  ignore(lines);
  ();
};

let update = (~bufferUpdate, ~lines, v: t) => {
  ignore(bufferUpdate);
  ignore(lines);
  v;
};

let getTokenColors= (v: t, line: int) => {
  ignore(v);
  ignore(line);
  [ColorizedToken2.create(
    ~index=0,
    ~backgroundColor=Revery.Colors.black,
    ~foregroundColor=Revery.Colors.red,
    ()),
    ColorizedToken2.create(
    ~index=10,
    ~backgroundColor=Revery.Colors.black,
    ~foregroundColor=Revery.Colors.green,
    ()),
    ColorizedToken2.create(
    ~index=20,
    ~backgroundColor=Revery.Colors.black,
    ~foregroundColor=Revery.Colors.blue,
    ()),
    ];
};
