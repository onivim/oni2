/* FilterJob.re
     FilterJob is a Job.t that describes how to break up the work of filtering
     items across multiple frames.
   */

module type Config = {
  type item;

  let format: item => string;
};

module Make = (Config: Config) => {
  open Oni_Core;

  open CamomileBundled.Camomile;
  module Zed_utf8 = Oni_Core.ZedBundled;
  module Time = Revery_Core.Time;

  let format = (item, ~shouldLower) => {
    let s = Config.format(item);
    shouldLower ? String.lowercase_ascii(s) : s;
  };

  module PendingWork = {
    type t = {
      filter: string,
      explodedFilter: list(UChar.t),
      shouldLower: bool,
      totalItemCount: int,
      allItems: Queue.t(list(Config.item)), // WARNING: mutable data structure
      itemsToFilter: Queue.t(list(Config.item)) // WARNING: mutable data structure
    };

    let create = () => {
      filter: "",
      allItems: Queue.create(),
      explodedFilter: [],
      shouldLower: false,
      itemsToFilter: Queue.create(),
      totalItemCount: 0,
    };

    let toString = ({allItems, itemsToFilter, totalItemCount, _}) =>
      Printf.sprintf(
        "- Pending Work\n -- totalItemCount: %n -- allItems: %n -- itemsToFilter: %n",
        totalItemCount,
        Queue.length(allItems),
        Queue.length(itemsToFilter),
      );
  };

  module CompletedWork = {
    type t = {
      allFiltered: list(Config.item),
      // If the allFiltered list is still huge,
      // we take a subset prior to sorting to display in the UI
      // The 'ui' filtered should be the main item for the UI to use
      uiFiltered: list(Filter.result(Config.item)),
    };

    let initial = {allFiltered: [], uiFiltered: []};

    let toString = ({allFiltered, uiFiltered}) =>
      Printf.sprintf(
        "- Completed Work\n -- allFiltered: %n -- uiFiltered: %n",
        List.length(allFiltered),
        List.length(uiFiltered),
      );
  };

  type t = Job.t(PendingWork.t, CompletedWork.t);

  // Constants
  let iterationsPerFrame = 250;
  let maxItemsToFilter = 250;

  /* [addItems] is a helper for `Job.map` that updates the job when the query has changed */
  let updateQuery =
      (
        filter,
        pending: PendingWork.t,
        {allFiltered, uiFiltered}: CompletedWork.t,
      ) => {
    // TODO: Optimize - for now, if the query changes, just clear the completed work
    // However, there are several ways we could improve this:
    // - If the query is just a stricter version... we could add the filter items back to completed
    // - If the query is broader, we could keep our current filtered items anyway
    let oldExplodedFilter = pending.explodedFilter;
    let explodedFilter = Zed_utf8.explode(filter);
    let shouldLower = filter == String.lowercase_ascii(filter);

    let currentMatches = Utility.firstk(maxItemsToFilter, allFiltered);

    // If the new query matches the old one... we can re-use results
    if (Filter.fuzzyMatches(oldExplodedFilter, filter)
        && List.length(currentMatches) < maxItemsToFilter) {
      let newPendingWork = {...pending, filter, explodedFilter, shouldLower};

      let newCompletedWork =
        CompletedWork.{
          allFiltered:
            List.filter(
              item =>
                Filter.fuzzyMatches(
                  explodedFilter,
                  format(item, ~shouldLower),
                ),
              allFiltered,
            ),
          uiFiltered:
            List.filter(
              (Filter.{item, _}) =>
                Filter.fuzzyMatches(
                  explodedFilter,
                  format(item, ~shouldLower),
                ),
              uiFiltered,
            ),
        };

      (false, newPendingWork, newCompletedWork);
    } else {
      let newPendingWork = {
        ...pending,
        filter,
        explodedFilter,
        itemsToFilter: Queue.copy(pending.allItems), // Reset items to filter
        shouldLower,
      };

      (false, newPendingWork, CompletedWork.initial);
    };
  };

  /* [addItems] is a helper for `Job.map` that updates the job when items have been added */
  let addItems = (items, pending: PendingWork.t, completed) => {
    Queue.push(items, pending.allItems);
    Queue.push(items, pending.itemsToFilter);

    let newPendingWork = {
      ...pending,
      totalItemCount: pending.totalItemCount + List.length(items),
    };

    (false, newPendingWork, completed);
  };

  /* [doWork] is run each frame until the work is completed! */
  let doActualWork =
      (pendingWork: PendingWork.t, {allFiltered, _}: CompletedWork.t) => {
    let rec loop = (i, current, completed) =>
      if (i >= iterationsPerFrame) {
        (false, completed);
      } else {
        switch (current) {
        | [item, ...rest] =>
          // Do a first filter pass to check if the item satisifies the regex
          let name = format(item, ~shouldLower=pendingWork.shouldLower);
          let matches = Filter.fuzzyMatches(pendingWork.explodedFilter, name);
          loop(i + 1, rest, matches ? [item, ...completed] : completed);

        | [] =>
          switch (Queue.take(pendingWork.itemsToFilter)) {
          | items => loop(i, items, completed)
          | exception Queue.Empty => (true, completed)
          }
        };
      };

    let (isComplete, completed) = loop(0, [], allFiltered);

    let uiFiltered =
      completed
      |> Utility.firstk(maxItemsToFilter)
      |> Filter.rank(pendingWork.filter, format);

    (
      isComplete,
      pendingWork,
      CompletedWork.{allFiltered: completed, uiFiltered},
    );
  };

  let doWork = (pending: PendingWork.t, completed) =>
    if (pending.filter == "") {
      let allFiltered =
        pending.allItems |> Queue.to_seq |> List.of_seq |> List.concat;
      let uiFiltered =
        allFiltered |> List.map(item => Filter.{highlight: [], item});
      (true, pending, CompletedWork.{allFiltered, uiFiltered});
    } else {
      doActualWork(pending, completed);
    };

  let progressReporter = (pending: PendingWork.t, _) => {
    let toFilter = float(Queue.length(pending.itemsToFilter));
    let total = float(Queue.length(pending.allItems));

    1.0 -. toFilter /. total;
  };

  let create = () => {
    Job.create(
      ~pendingWorkPrinter=PendingWork.toString,
      ~completedWorkPrinter=CompletedWork.toString,
      ~progressReporter,
      ~name="FilterJob",
      ~initialCompletedWork=CompletedWork.initial,
      ~budget=Time.ms(2),
      ~f=doWork,
      PendingWork.create(),
    );
  };
};
