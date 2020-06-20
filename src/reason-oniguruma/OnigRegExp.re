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

external _create: string => result(t, string) = "reonig_create";
external _search: (string, int, t) => array(Match.t) = "reonig_search";
external _end: unit => unit = "reonig_end";

let create = (re: string) => {
  _create(re);
};

let search = (str: string, startPosition: int, regexp: t) => {
  _search(str, startPosition, regexp);
};

let test = (str: string, regexp: t) => {
  let matches = _search(str, 0, regexp);
  Array.length(matches) > 0;
};

at_exit(_end);
