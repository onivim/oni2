/* FilterJob.re
     FilterJob is a Job.t that describes how to break up the work of filtering
     items across multiple frames.
   */
open Oni_Core;

module Constants = {
  let itemsPerFrame = 250;
  let maxItemsToFilter = 250;
};

module type Config = {
  type item;

  let format: item => string;
};

module Make = (Config: Config) => {
  open CamomileBundled.Camomile;
  module Zed_utf8 = Oni_Core.ZedBundled;
  module Time = Revery_Core.Time;
  module Queue = Utility.ChunkyQueue;

  let format = (item, ~shouldLower) => {
    let s = Config.format(item);
    shouldLower ? String.lowercase_ascii(s) : s;
  };

  module PendingWork = {
    type t = {
      filter: string,
      explodedFilter: list(UChar.t),
      shouldLower: bool,
      allItems: Queue.t(Config.item),
      queue: Queue.t(Config.item),
    };

    let create = () => {
      filter: "",
      allItems: Queue.empty,
      explodedFilter: [],
      shouldLower: false,
      queue: Queue.empty,
    };

    let toString = ({allItems, queue, _}) =>
      Printf.sprintf(
        "- Pending Work\n -- allItems: %n -- itemsToFilter: %n",
        Queue.length(allItems),
        Queue.length(queue),
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

    let currentMatches =
      Utility.firstk(Constants.maxItemsToFilter, allFiltered);

    // If the new query matches the old one... we can re-use results
    if (pending.filter != ""
        && Filter.fuzzyMatches(oldExplodedFilter, filter)
        && List.length(currentMatches) < Constants.maxItemsToFilter) {
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
        queue: pending.allItems, // Reset items to filter
        shouldLower,
      };

      (false, newPendingWork, CompletedWork.initial);
    };
  };

  /* [addItems] is a helper for `Job.map` that updates the job when items have been added */
  let addItems = (items, pending: PendingWork.t, completed) => {
    let newPendingWork = {
      ...pending,
      allItems: Queue.pushReversedChunk(items, pending.allItems),
      queue: Queue.pushReversedChunk(items, pending.queue),
    };

    (false, newPendingWork, completed);
  };

  let doActualWork =
      (
        {queue, shouldLower, filter, explodedFilter, _} as pendingWork: PendingWork.t,
        {allFiltered, _}: CompletedWork.t,
      ) => {
    // Take out the items to process this frame
    let (items, queue) = Queue.take(Constants.itemsPerFrame, queue);

    // Do a first filter pass to check if the item satisifies the regex
    let allFiltered =
      List.fold_left(
        (completed, item) => {
          let name = format(item, ~shouldLower);

          if (Filter.fuzzyMatches(explodedFilter, name)) {
            [item, ...completed];
          } else {
            completed;
          };
        },
        allFiltered,
        items,
      );

    // Rank a limited nuumber of filtered items
    let uiFiltered =
      allFiltered
      |> Utility.firstk(Constants.maxItemsToFilter)
      |> Filter.rank(filter, format);

    (
      Queue.isEmpty(queue),
      {...pendingWork, queue},
      CompletedWork.{allFiltered, uiFiltered},
    );
  };

  /* [doWork] is run each frame until the work is completed! */
  let doWork = (pending: PendingWork.t, completed) =>
    if (pending.filter == "") {
      let allFiltered = pending.allItems |> Queue.toList;
      let uiFiltered =
        allFiltered |> List.map(item => Filter.{highlight: [], item});
      (true, pending, CompletedWork.{allFiltered, uiFiltered});
    } else {
      doActualWork(pending, completed);
    };

  let progressReporter = (pending: PendingWork.t, _) => {
    let toFilter = Queue.length(pending.queue);
    let total = Queue.length(pending.allItems);

    1.0 -. float(toFilter) /. float(total);
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
