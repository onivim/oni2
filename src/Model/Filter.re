/*
 * Filter.re
 *
 * Module to filter & rank items using various strategies.
 */
open Actions;
open ReasonFuzz;

module Option = {
  let map = f => fun
    | Some(x) => Some(f(x))
    | None => None 
}

let scoreObject = ((match, menuItem)) =>
  (
    Option.map(m => MatchResult.create(m.IndexMatchResult.score), match),
    menuItem.name
  );

let compareScore = (x, y) =>
  compareScores(scoreObject(x), scoreObject(y));

let ranges = indices =>
  // Assumes the array is sorted low to high
  Array.fold_left(
    (acc, i) =>
      switch acc {
        | [] =>
          [(i, i)]

        | [(low, high), ...rest] =>
          if (high + 1 == i) {
            [(low, i), ...rest] // Extend current range
          } else {
            [(i, i), ...acc] // Add new range
          }
      }, [], indices)
    |> List.rev;

let makeResult = ((maybeMatch, menuItem)) =>
  switch (maybeMatch) {
    | Some(match) =>
      { ...menuItem, highlight: ranges(match.IndexMatchResult.indicies) }
    | None =>
      menuItem
  };

let formatName = (item, ~shouldLower) => {
  let label =
    Quickmenu.getLabel(item);

  if (shouldLower) {
    String.lowercase_ascii(label)
  } else {
    label
  };
}

let processItem = (pattern, ~shouldLower, item) => {
  let line =
    formatName(item, ~shouldLower);
  let match =
    pathIndexMatch(~line, ~pattern);

  (match, item)
};


let rank = (query, items) => {
  /* Use smart search for now, add config option though. */
  let shouldLower =
    query == String.lowercase_ascii(query);

  items
    |> List.map(processItem(query, ~shouldLower))
    |> List.sort(compareScore)
    |> List.map(makeResult);
};
