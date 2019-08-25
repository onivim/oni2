/*
 * SyntaxHighlighting2.re
 *
 * State kept for syntax highlighting (via TextMate today)
 */
open Oni_Core;
open Oni_Core.Types;
open Oni_Syntax.SyntaxHighlights;

type t = IntMap.t(SyntaxHighlights.t);

let empty = IntMap.empty;

let getTokensForLine = (v: t, bufferId: int, line: int) => {
  //print_endline ("getTokensForLine: " ++ string_of_int(bufferId));
  switch (IntMap.find_opt(bufferId, v)) {
  | Some(v) =>
    TestSyntaxHighlight.getTokenColors(v.testSyntaxHighlights, line)
  | None => []
  };
};
