/*
 * Filter.re
 *
 * Module to filter & rank items using various strategies.
 */
open Actions;

let compareScore =
    (
      item1: (option(ReasonFuzz.MatchResult.t), Actions.menuCommand),
      item2: (option(ReasonFuzz.MatchResult.t), Actions.menuCommand),
    ) => {
  let scoreObj1 = (fst(item1), snd(item1).name);
  let scoreObj2 = (fst(item2), snd(item2).name);

  ReasonFuzz.compareScores(scoreObj1, scoreObj2);
};

let formatName = (item, shouldLower) =>
  switch (item.category, shouldLower) {
  | (Some(c), true) => String.lowercase_ascii(c ++ item.name)
  | (Some(c), false) => c ++ item.name
  | (None, true) => String.lowercase_ascii(item.name)
  | (None, false) => item.name
  };

let rank = (query, items) => {
  /* Use smart search for now, add config option though. */
  let shouldLower = query == String.lowercase_ascii(query);

  let scoreList =
    List.map(
      item =>
        (
          ReasonFuzz.pathFuzzyMatch(
            ~line=formatName(item, shouldLower),
            ~pattern=query,
          ),
          item,
        ),
      items,
    );

  let sortedList =
    List.sort((item1, item2) => compareScore(item1, item2), scoreList);

  List.map(i => snd(i), sortedList);
};
