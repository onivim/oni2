/*
 * Filter.re
 *
 * Module to filter & rank items using various strategies.
 */
open ReasonFuzz;

open Oni_Core.CamomileBundled.Camomile;

module Utility = Oni_Core.Utility;
module Zed_utf8 = Oni_Core.ZedBundled;

module Option = {
  let map = f =>
    fun
    | Some(x) => Some(f(x))
    | None => None;
};

type result('a) = {
  item: 'a,
  highlight: list((int, int)),
};

let makeResult = ((item, match: Fzy.Result.t)) => {
      item,
      highlight: Utility.ranges(match.positions),
  };

let rank = (query, format, items) => {
  let shouldLower = query == String.lowercase_ascii(query);
  let format = item => format(item, ~shouldLower);

  let search = (query, format, item) => {
    Console.log("Starting search...");
    let searchStr = format(item);
    Console.log("Checking " ++ searchStr ++ " against " ++ query);
    let result = Fzy.fzySearchList([searchStr], query);
    Console.log("Returning from search...");
    Console.log(result);

    (item, List.hd(result))
  }

  items
  |> List.map(search(query, format))
  |> List.map(makeResult);
};

// Check whether the query matches...
// Benchmarking showed that this was slightly faster than the recursive version
let fuzzyMatches = (query: list(UChar.t), str) => {
  let toMatch = Zed_utf8.explode(str);

  let q = ref(query);
  let m = ref(toMatch);

  let atEnd = ref(false);
  let result = ref(false);

  while (! atEnd^) {
    switch (q^, m^) {
    | ([], _) =>
      result := true;
      atEnd := true;
    | (_, []) =>
      result := false;
      atEnd := true;
    | ([qh, ...qtail], [mh, ...mtail]) when UChar.eq(qh, mh) =>
      q := qtail;
      m := mtail;
    | (_, [_, ...mtail]) => m := mtail
    };
  };

  result^;
};
