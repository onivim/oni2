/*
 RegExp.re

 A wrapper around Oniguruma regexps for the purpose of syntax highlighting.

 There are some cases where the raw regular expression isn't enough - for example,
 when using back-references in capture groups - these need to be generated on the fly.
 */

open Oniguruma;

type cachedResult = 
| NotEvaluated
| EvaluatedPosition({ str: string, position: int })
| EvaluatedMatches({ str: string, position: int, matches: array(OnigRegExp.Match.t)});

type t = {
  mutable cachedResult: cachedResult,
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
    | EvaluatedPosition({ str, position })
    | EvaluatedMatches({ str, position, _}) when (position >= positionToEvaluate || position == -1) && String.equal(str, stringToEvaluate) =>
      position
    | _ =>
      let newResult = OnigRegExp.Fast.search(stringToEvaluate, positionToEvaluate, v.regexp);
      v.cachedResult = EvaluatedPosition({ str: stringToEvaluate, position: newResult });
      newResult;
    };
};

let matches = (v: t) => {
    switch (v.cachedResult) {
    | NotEvaluated => emptyMatches
    | EvaluatedMatches({ matches, _}) => matches
    | EvaluatedPosition({ str, position }) =>
      let matches = OnigRegExp.Fast.getLastMatches(str, v.regexp);
      v.cachedResult = EvaluatedMatches({ str, position, matches });
      matches;
    }
};
