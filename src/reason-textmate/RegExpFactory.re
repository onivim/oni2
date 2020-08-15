/*
 RegExpFactory.re

 A wrapper around RegExp, to handle anchors like \\A and \\G.
 */

type captureGroup = (int, string);

type anchorCache = {
  raw_A0_G0: string,
  raw_A1_G0: string,
  raw_A0_G1: string,
  raw_A1_G1: string,
};

// If there are no back references, we can cache the
// compiled anchor caches, too
type compiledAnchorCache = {
  a0_G0: RegExp.t,
  a1_G0: RegExp.t,
  a0_G1: RegExp.t,
  a1_G1: RegExp.t,
};

type t = {
  hasAnchorA: bool,
  hasAnchorG: bool,
  hasUnresolvedBackReferences: bool,
  captureGroups: option(list(captureGroup)),
  raw: string,
  anchorCache: option(anchorCache),
  // If the regex doesn't have anchors,
  // we can just keep a ready-to-go version around.
  regex: option(RegExp.t),
  // If no back references, can keep the compiled anchor caches around too
  compiledAnchorCache: option(compiledAnchorCache),
};

let hasAnchorA = Str.regexp("\\\\A");
let hasAnchorG = Str.regexp("\\\\G");
let hasAnchors = (v: t) => v.hasAnchorA || v.hasAnchorG;

let hasBackRefRegExp = Str.regexp("\\\\\\([0-9]+\\)");
let hasBackReferences = (v: t) => v.hasUnresolvedBackReferences;

let charactersToEscape = Str.regexp("[\\?\\,\\.\\$\\^\\+\\*{}\\\\\\|\\-]");
let additionalCharactersToEscape = Str.regexp("[][()]");
let escapeRegExpCharacters = (str: string) => {
  let f = s => "\\" ++ s;

  str
  |> Str.global_substitute(charactersToEscape, f)
  |> Str.global_substitute(additionalCharactersToEscape, f);
};

let _createAnchorCache = (str: string) => {
  let f = _ => "\\uFFFF";

  let raw_A0_G0 =
    str
    |> Str.global_substitute(hasAnchorA, f)
    |> Str.global_substitute(hasAnchorG, f);

  let raw_A0_G1 = Str.global_substitute(hasAnchorA, f, str);

  let raw_A1_G0 = Str.global_substitute(hasAnchorG, f, str);

  let raw_A1_G1 = str;

  Some({raw_A0_G1, raw_A1_G1, raw_A0_G0, raw_A1_G0});
};

let _createCompiledAnchorCache = (ac: option(anchorCache)) => {
  switch (ac) {
  | None => None
  | Some(v) =>
    let a0_G0 = RegExp.create(v.raw_A0_G0);
    let a0_G1 = RegExp.create(v.raw_A0_G1);
    let a1_G1 = RegExp.create(v.raw_A1_G1);
    let a1_G0 = RegExp.create(v.raw_A1_G0);
    Some({a0_G0, a0_G1, a1_G1, a1_G0});
  };
};

let create = (~allowBackReferences=true, str) => {
  // We allow some regular expressions to have backreferences -
  // for example, 'begin' and 'match' rules can have them.
  // However, 'end' rules need the matches from the 'begin'
  // to be plugged-in.
  let hasUnresolvedBackReferences =
    !allowBackReferences
    && (
      switch (Str.search_forward(hasBackRefRegExp, str, 0)) {
      | exception _ => false
      | _ => true
      }
    );

  let anchorA =
    switch (Str.search_forward(hasAnchorA, str, 0)) {
    | exception _ => false
    | _ => true
    };

  let anchorG =
    switch (Str.search_forward(hasAnchorG, str, 0)) {
    | exception _ => false
    | _ => true
    };

  // If no back-references, and no anchors, we can just cache the regex
  let regex =
    if (!hasUnresolvedBackReferences && !anchorA && !anchorG) {
      Some(RegExp.create(str));
    } else {
      None;
    };

  let (anchorCache, compiledAnchorCache) =
    if (anchorA || anchorG) {
      let anchorCache = _createAnchorCache(str);

      let compiledAnchorCache =
        if (!hasUnresolvedBackReferences) {
          _createCompiledAnchorCache(anchorCache);
        } else {
          None;
        };
      (anchorCache, compiledAnchorCache);
    } else {
      (None, None);
    };

  {
    captureGroups: None,
    raw: str,
    regex,
    anchorCache,
    compiledAnchorCache,
    hasAnchorA: anchorA,
    hasAnchorG: anchorG,
    hasUnresolvedBackReferences,
  };
};

let supplyReferences = (references: list(captureGroup), v: t) => {
  let newRawStr =
    List.fold_left(
      (prev, curr) => {
        let (cg, text) = curr;
        let str = prev;

        let newStr =
          if (cg > 0) {
            let regexp = Str.regexp("\\\\" ++ string_of_int(cg));
            let text = escapeRegExpCharacters(text);
            Str.global_replace(regexp, text, str);
          } else {
            prev;
          };

        newStr;
      },
      v.raw,
      references,
    );

  create(newRawStr);
};

let compile = (allowA, allowG, v: t) =>
  if (v.hasUnresolvedBackReferences) {
    let rawStr =
      switch (v.anchorCache, allowA, allowG) {
      | (None, _, _) => v.raw
      | (Some({raw_A1_G1, _}), allowA, allowG)
          when allowA == true && allowG == true => raw_A1_G1
      | (Some({raw_A1_G0, _}), allowA, _) when allowA == true => raw_A1_G0
      | (Some({raw_A0_G1, _}), _, allowG) when allowG == true => raw_A0_G1
      | (Some({raw_A0_G0, _}), _, _) => raw_A0_G0
      };
    RegExp.create(rawStr);
  } else {
    switch (v.regex) {
    | Some(v) => v
    | None =>
      switch (v.compiledAnchorCache, allowA, allowG) {
      | (None, _, _) => failwith("Should never hit this!")

      | (Some({a1_G1, _}), allowA, allowG)
          when allowA == true && allowG == true => a1_G1
      | (Some({a1_G0, _}), allowA, _) when allowA == true => a1_G0
      | (Some({a0_G1, _}), _, allowG) when allowG == true => a0_G1
      | (Some({a0_G0, _}), _, _) => a0_G0
      }
    };
  };

let show = (v: t) => v.raw;
