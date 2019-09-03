/*
 TextMateGrammar.re
 */

open Oni_Core;

module Capture = {
  type t = (int, string);
};

type pattern =
| Include(string)
| Match(match)
| MatchRange(matchRange)
and match = {
    matchRegex: string,
    matchName: string,
    captures: list(Capture.t),
} and matchRange = {
    beginRegex: string,
    endRegex: string,
    beginCaptures: list(Capture.t),
    endCaptures: list(Capture.t),
    matchRangeName: string,
    patterns: list(pattern)
};

type t = {
  scopeName: string,
  patterns: list(pattern),
  repository: StringMap.t(pattern),
}
