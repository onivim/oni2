/*
 RegExp.re

 A wrapper around Oniguruma regexps for the purpose of syntax highlighting.

 There are some cases where the raw regular expression isn't enough - for example,
 when using back-references in capture groups - these need to be generated on the fly.
 */

open Oniguruma;

type cachedResult =
  | NotEvaluated
  | EvaluatedPosition({
      str: string,
      evaluatedPosition: int,
      matchPosition: int,
    })
  | EvaluatedMatches({
      str: string,
      matchPosition: int,
      evaluatedPosition: int,
      matches: array(OnigRegExp.Match.t),
    });

type t = {
  mutable cachedResult,
  raw: string,
  regexp: OnigRegExp.t,
};

let raw = (v: t) => v.raw;
let toString = (v: t) => v.raw;

let emptyMatches = [||];

let create = (str: string) => {
  let regexp =
    switch (OnigRegExp.create(str)) {
    | Ok(v) => v
    | Error(msg) => failwith(msg)
    };

  {cachedResult: NotEvaluated, raw: str, regexp};
};

let search = (stringToEvaluate: string, positionToEvaluate: int, v: t) => {
  switch (v.cachedResult) {
  | EvaluatedPosition({str, matchPosition, evaluatedPosition})
  | EvaluatedMatches({str, matchPosition, evaluatedPosition, _})
      // We already have a match past this position, so we already evaluated:
      when
        (
          matchPosition >= positionToEvaluate
          // or we've evaluated before this position, and we didn't get a match:
          || evaluatedPosition <= positionToEvaluate
          && matchPosition == (-1)
        )
        // and the string is equal to the one we tested before - return previous match
        && String.equal(str, stringToEvaluate) => matchPosition
  | _ =>
    let newResult =
      OnigRegExp.Fast.search(stringToEvaluate, positionToEvaluate, v.regexp);
    v.cachedResult =
      EvaluatedPosition({
        str: stringToEvaluate,
        evaluatedPosition: positionToEvaluate,
        matchPosition: newResult,
      });
    newResult;
  };
};

let matches = (v: t) => {
  switch (v.cachedResult) {
  | NotEvaluated => emptyMatches

  | EvaluatedMatches({matches, _}) => matches

  // If no matches, don't bother calling out to Oniguruma FFI - we know we have nothing...
  | EvaluatedPosition({matchPosition, _}) when matchPosition == (-1) => emptyMatches

  | EvaluatedPosition({str, matchPosition, evaluatedPosition}) =>
    let matches = OnigRegExp.Fast.getLastMatches(str, v.regexp);
    v.cachedResult =
      EvaluatedMatches({str, matchPosition, matches, evaluatedPosition});
    matches;
  };
};
