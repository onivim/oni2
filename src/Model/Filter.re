/*
 * Filter.re
 *
 * Module to filter & rank items using various strategies.
 */
open ReasonFuzz;

module Utility = Oni_Core.Utility;

module Option = {
  let map = f => fun
    | Some(x) => Some(f(x))
    | None => None 
}

type result('a) = {
  item: 'a,
  highlight: list((int, int))
}

let makeResult = ((maybeMatch, item)) =>
  switch (maybeMatch) {
    | Some(match) =>
      { item, highlight: Utility.ranges(match.IndexMatchResult.indicies) }
    | None =>
      { item, highlight: [] }
  };


let rank = (query, format, items) => {
  let shouldLower =
    query == String.lowercase_ascii(query);
  let format = item =>
    format(item, ~shouldLower);

  let compareScore = (x, y) => {
    let scoreObject = ((match, item)) =>
      (
        Option.map(m => MatchResult.create(m.IndexMatchResult.score), match),
        format(item)
      );
    compareScores(scoreObject(x), scoreObject(y));
  };

  let processItem = (pattern, item) => {
    let line =
      format(item);
    let match =
      pathIndexMatch(~line, ~pattern);

    (match, item)
  };

  items
    |> List.map(processItem(query))
    |> List.sort(compareScore)
    |> List.map(makeResult);
};