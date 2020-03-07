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

let makeResult = ((maybeMatch, item)) =>
  switch (maybeMatch) {
  | Some(match) => {
      item,
      highlight: IndexEx.ranges(match.IndexMatchResult.indicies),
    }
  | None => {item, highlight: []}
  };

let rank = (query, format, items) => {
  let shouldLower = query == String.lowercase_ascii(query);
  let format = item => format(item, ~shouldLower);

  let compareScore = (x, y) => {
    let scoreObject = ((match, item)) => (
      Option.map(m => MatchResult.create(m.IndexMatchResult.score), match),
      format(item),
    );
    compareScores(scoreObject(x), scoreObject(y));
  };

  let processItem = (pattern, item) => {
    let line = format(item);
    let match = pathIndexMatch(~line, ~pattern);

    (match, item);
  };

  items
  |> List.map(processItem(query))
  |> List.sort(compareScore)
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
