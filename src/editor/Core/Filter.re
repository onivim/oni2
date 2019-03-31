/**
   Filter

   Module to handle filtering of items using various strategies
 */

let _compareScore =
    (
      item1: (option(ReasonFuzz.MatchResult.t), Types.UiMenu.command),
      item2: (option(ReasonFuzz.MatchResult.t), Types.UiMenu.command),
    ) => {
  let scoreObj1 = (fst(item1), snd(item1).name);
  let scoreObj2 = (fst(item2), snd(item2).name);

  ReasonFuzz.compareScores(scoreObj1, scoreObj2);
};

let menu = (query, items) => {
  let scoreList =
    Types.UiMenu.(
      List.map(
        item1 =>
          (
            ReasonFuzz.pathFuzzyMatch(~line=item1.name, ~pattern=query),
            item1,
          ),
        items,
      )
    );

  let sortedList =
    List.sort((item1, item2) => _compareScore(item1, item2), scoreList);
  List.map(i => snd(i), sortedList);
};
