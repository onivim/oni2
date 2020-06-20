type t;

type rawMatch = (int, int);

module Match = {
  type t = {
    index: int,
    startPos: int,
    length: int,
    endPos: int,
    str: string,
  };

  let getText = (v: t) => {
    String.sub(v.str, v.startPos, v.length);
  };
};

module Bindings = {
external create: string => result(t, string) = "reonig_create";
external search: (string, int, t) => array(Match.t) = "reonig_search";
external finalize: unit => unit = "reonig_end";

external search_fast: (string, int, t) => int = "reonig_search_fast";
external get_last_matches: (string, t) => array(Match.t) = "reonig_get_last_matches";
}

let create = (re: string) => {
  Bindings.create(re);
};

let search = (str: string, startPosition: int, regexp: t) => {
  Bindings.search(str, startPosition, regexp);
};


module Fast = {
  let search = (str, startPosition, regexp) => {
    Bindings.search_fast(str, startPosition, regexp);
  }

  let getLastMatches = (str, regexp) => {
    Bindings.get_last_matches(str, regexp);
  };

  let test = (str, regexp) => {
    Bindings.search_fast(str, 0, regexp) >= 0;
  };
}

let test = Fast.test;

at_exit(Bindings.finalize);
