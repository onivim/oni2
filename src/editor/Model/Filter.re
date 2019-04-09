/**
   Filter

   Module to handle filtering of items using various strategies
 */
open Oni_Core;
open Actions;

let menu = (query, items: list(Actions.menuCommand(State.t))) =>
  List.sort(
    (item1, item2) => {
      let firstMatches = Utility.stringContains(item1.name, query);
      let secondMatches = Utility.stringContains(item2.name, query);
      /**
           if both values are the same then no position change is required
           if the first item is a match then move it closer to the start
           if the second is a match move the first lower down
         */
      (
        switch (firstMatches, secondMatches) {
        | (true, true) => 0
        | (false, false) => 0
        | (true, false) => (-1)
        | (false, true) => 1
        }
      );
    },
    items,
  );
