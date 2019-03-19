/**
   Search

   Module to handle searching and filtering of items using
   various strategies
 */
open Oni_Core;

let menu = (query, items) =>
  Types.UiMenu.(
    List.sort(
      (item1, item2) => {
        let firstMatches = Utility.contains(item1.name, query);
        let secondMatches = Utility.contains(item2.name, query);
        switch (firstMatches, secondMatches) {
        | (true, true) => 0
        | (false, false) => 0
        | (true, false) => (-1)
        | (false, true) => 1
        };
      },
      items,
    )
  );
