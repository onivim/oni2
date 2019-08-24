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

let formatName = (item, shouldLower) => {
  let itemName =
    switch (item.category) {
    | Some(c) => c ++ item.name
    | None => item.name
    };

  if (shouldLower) {
    String.lowercase_ascii(itemName);
  } else {
    itemName;
  };
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
