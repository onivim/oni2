/*
 * Filter.re
 *
 * Module to filter & rank items using various strategies.
 */
open ReasonFuzz;

module IndexEx = Utility.IndexEx;

type result('a) = {
  item: 'a,
  highlight: list((int, int)),
};

let makeResult = ((item, match: Fzy.Result.t)) => {
  item,
  highlight: IndexEx.ranges(match.positions),
};

let rank = (query, format, items) => {
  let shouldLower = query == String.lowercase_ascii(query);
  let format = item => format(item, ~shouldLower);

  let search = (query, format, items) => {
    let searchStrings = List.map(item => format(item), items);
    let results = Fzy.fzySearchList(searchStrings, query, ());
    let lookup = (result: Fzy.Result.t) => List.nth(items, result.original_index);

    List.map((result) => (lookup(result), result), results);
  };

  items
  |> search(query, format)
  |> List.map(makeResult);
};

// Check whether the query matches...
// Benchmarking showed that this was slightly faster than the recursive version
let fuzzyMatches = (query: list(Uchar.t), str) => {
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
    | ([qh, ...qtail], [mh, ...mtail]) when Uchar.equal(qh, mh) =>
      q := qtail;
      m := mtail;
    | (_, [_, ...mtail]) => m := mtail
    };
  };

  result^;
};
