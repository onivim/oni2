open EditorCoreTypes;

type passThroughCharacter = string;

module AutoPair = {
  type t = {
    opening: string,
    closing: string,
  };
};

type t = {
  pairs: list(AutoPair.t),
  deletionPairs: list(AutoPair.t),
  passThrough: list(string),
  before: Hashtbl.t(string, bool),
};

let empty: t = {
  passThrough: [],
  deletionPairs: [],
  pairs: [],
  before: Hashtbl.create(0),
};

let create =
    (~allowBefore=[], ~passThrough=?, ~deletionPairs=?, p: list(AutoPair.t)) => {
  let deletionPairs = Option.value(deletionPairs, ~default=p);

  let passThrough =
    Option.value(
      passThrough,
      ~default=List.map((pair: AutoPair.t) => pair.closing, p),
    );

  let hash = Hashtbl.create(16);
  List.iter(item => Hashtbl.add(hash, item, true), allowBefore);
  {deletionPairs, pairs: p, before: hash, passThrough};
};

let isMatchingPair = (c, closingPairs: t) => {
  List.exists((p: AutoPair.t) => p.opening == c, closingPairs.pairs);
};

let isOpeningPair = (c, closingPairs: t) => {
  List.exists((p: AutoPair.t) => p.opening == c, closingPairs.pairs);
};

let isClosingPair = (c, closingPairs: t) => {
  List.exists((p: AutoPair.t) => p.closing == c, closingPairs.pairs);
};

let isPassThroughPair = (c, closingPairs: t) => {
  List.exists(str => str == c, closingPairs.passThrough);
};

let getByOpeningPair = (c, closingPairs: t) => {
  let matches =
    List.filter((p: AutoPair.t) => p.opening == c, closingPairs.pairs);

  switch (matches) {
  | [hd, ..._] => hd
  | [] => failwith("No matching pair: " ++ c)
  };
};

let isBetweenPairs = (line, index, pairs) => {
  let index = Index.toZeroBased(index);
  let len = String.length(line);
  if (index > 0 && index < len) {
    List.exists(
      (p: AutoPair.t) =>
        p.opening == String.sub(line, index - 1, 1)
        && p.closing == String.sub(line, index, 1),
      pairs,
    );
  } else {
    false;
  };
};

let isBetweenClosingPairs = (line, index, closingPairs) => {
  isBetweenPairs(line, index, closingPairs.pairs);
};

let isBetweenDeletionPairs = (line, index, closingPairs) => {
  isBetweenPairs(line, index, closingPairs.deletionPairs);
};

let _exists = (key, hashtbl) =>
  switch (Hashtbl.find_opt(hashtbl, key)) {
  | Some(_) => true
  | None => false
  };

let canCloseBefore = (line, index, closingPairs) => {
  let index = Index.toZeroBased(index);
  let len = String.length(line);
  if (index > 0 && index < len) {
    let nextChar = String.sub(line, index, 1);
    _exists(nextChar, closingPairs.before);
  } else {
    true;
        // No character past cursor, always allow
  };
};

let doesNextCharacterMatch = (line, index, s) => {
  let index = Index.toZeroBased(index);
  let len = String.length(line);
  if (index > 0 && index < len) {
    s == String.sub(line, index, 1);
  } else {
    false;
  };
};

let isPassThrough = (character, line, index, autoClosingPairs) => {
  isPassThroughPair(character, autoClosingPairs)
  && doesNextCharacterMatch(line, index, character);
};
