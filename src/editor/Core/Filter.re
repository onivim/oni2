/**
   Filter

   Module to handle filtering of items using various strategies
 */

let menu = (query, items) => {
  Console.log("Query is " ++ query);
  Console.log("Len of list is " ++ string_of_int(List.length(items)));

  Types.UiMenu.(
    List.sort(
      (item1, item2) => {
        let firstMatchResult = ReasonFuzz.pathFuzzyMatch(~line=item1.name, ~pattern=query);
        let secondMatchResult = ReasonFuzz.pathFuzzyMatch(~line=item2.name, ~pattern=query);

        let (firstScore, secondScore) = switch ((firstMatchResult, secondMatchResult)) {
        | (Some(match1), Some(match2)) => (match1.score, match2.score)
        | (None, Some(match2)) => (-9999999999, match2.score)
        | (Some(match1), None) => (match1.score, -9999999999)
        | _ => (-9999999999, -9999999999)
        };

        compare(firstScore, secondScore) * -1
      },
      items,
    )
  );
}