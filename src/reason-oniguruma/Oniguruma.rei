module OnigRegExp: {
  type t;

  module Match: {
    type t = {
      index: int,
      startPos: int,
      length: int,
      endPos: int,
      str: string,
    };

    let getText: t => string;
  };

  let create: string => result(t, string);
  let search: (string, int, t) => array(Match.t);
  let test: (string, t) => bool;

  module Fast: {
    let search: (string, int, t) => int;
    let getLastMatches: (string, t) => array(Match.t);
    let test: (string, t) => bool;
  };
};
