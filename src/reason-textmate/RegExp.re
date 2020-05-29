/*
 RegExp.re

 A wrapper around Oniguruma regexps for the purpose of syntax highlighting.

 There are some cases where the raw regular expression isn't enough - for example,
 when using back-references in capture groups - these need to be generated on the fly.
 */

open Oniguruma;

let _allowCache = ref(true);

let setAllowCache = v => _allowCache := v;

type cachedResult = {
  str: string,
  position: int,
  matches: array(OnigRegExp.Match.t),
};

type t = {
  mutable cachedResult: option(cachedResult),
  raw: string,
  regexp: option(OnigRegExp.t),
};

let raw = (v: t) => v.raw;
let toString = (v: t) => v.raw;

let emptyMatches = [||];

let create = (str: string) => {
  let regexp =
    switch (OnigRegExp.create(str)) {
    | Ok(v) => Some(v)
    | Error(msg) => failwith(msg)
    };

  {cachedResult: None, raw: str, regexp};
};

let search = (str: string, position: int, v: t) => {
  let run = () => {
    switch (v.regexp) {
    | Some(re) => OnigRegExp.search(str, position, re)
    | None => emptyMatches
    };
  };

  if (! _allowCache^) {
    run();
  } else {
    switch (v.cachedResult) {
    | Some(cachedResult)
        when cachedResult.position >= position && cachedResult.str === str =>
      cachedResult.matches
    | _ =>
      let newResult = run();
      let len = Array.length(newResult);
      if (len >= 1) {
        v.cachedResult =
          Some({str, position: newResult[0].startPos, matches: newResult});
      } else {
        v.cachedResult = None;
      };
      newResult;
    };
  };
};
