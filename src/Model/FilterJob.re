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

  let format = (item, ~shouldLower) => {
    let s = Config.format(item);
    shouldLower ? String.lowercase_ascii(s) : s;
  };

  type pendingWork = {
    filter: string,
    // Full commands is the _complete set_ of unfiltered commands
    // This never gets filtered - it's persisted in case we need
    // the full set again
    explodedFilter: list(UChar.t),
    shouldLower: bool,
    totalItemCount: int,
    allItems: list(list(Config.item)),
    // Commands to filter are commands we haven't looked at yet.
    itemsToFilter: list(list(Config.item)),
  };

  let showPendingWork = (v: pendingWork) => {
    "- Pending Work\n"
    ++ " -- totalItemCount: "
    ++ string_of_int(v.totalItemCount)
    ++ " -- allItems: "
    ++ string_of_int(List.length(v.allItems))
    ++ " -- itemsToFilter: "
    ++ string_of_int(List.length(v.itemsToFilter));
  };

  type completedWork = {
    allFiltered: list(Config.item),
    // If the allFiltered list is still huge,
    // we take a subset prior to sorting to display in the UI
    // The 'ui' filtered should be the main item for the UI to use
    uiFiltered: list(Filter.result(Config.item)),
  };

  let showCompletedWork = (v: completedWork) => {
    "- Completed Work\n"
    ++ " -- allFiltered: "
    ++ string_of_int(List.length(v.allFiltered))
    ++ " -- uiFiltered: "
    ++ string_of_int(List.length(v.uiFiltered));
  };

  type t = Job.t(pendingWork, completedWork);

  let initialCompletedWork = {allFiltered: [], uiFiltered: []};
  let initialPendingWork = {
    filter: "",
    allItems: [],
    explodedFilter: [],
    shouldLower: false,
    itemsToFilter: [],
    totalItemCount: 0,
  };

  // Constants
  let iterationsPerFrame = 250;
  let maxItemsToFilter = 250;

  // Check whether the query matches...
  // Benchmarking showed that this was slightly faster than the recursive version
  let matches = (query: list(UChar.t), str) => {
    let toMatch = Zed_utf8.explode(str);

    let q = ref(query);
    let m = ref(toMatch);

    let atEnd = ref(false);
    let result = ref(false);

    while (! atEnd^) {
      switch (q^, m^) {
      | ([], _) =>
        result := true;
        atEnd := true;
      | (_, []) =>
        result := false;
        atEnd := true;
      | ([qh, ...qtail], [mh, ...mtail]) when UChar.eq(qh, mh) =>
        q := qtail;
        m := mtail;
      | (_, [_, ...mtail]) => m := mtail
      };
    };

    result^;
  };

  /* [addItems] is a helper for `Job.map` that updates the job when the query has changed */
  let updateQuery = (newQuery: string, p: pendingWork, c: completedWork) => {
    // TODO: Optimize - for now, if the query changes, just clear the completed work
    // However, there are several ways we could improve this:
    // - If the query is just a stricter version... we could add the filter items back to completed
    // - If the query is broader, we could keep our current filtered items anyway
    let newQueryEx = Zed_utf8.explode(newQuery);
    let shouldLower = newQuery == String.lowercase_ascii(newQuery);

    let currentMatches = Utility.firstk(maxItemsToFilter, c.allFiltered);

    // If the new query matches the old one... we can re-use results
    if (matches(p.explodedFilter, newQuery)
        && List.length(currentMatches) < maxItemsToFilter) {
      let {allFiltered, uiFiltered} = c;

      let uiFilteredNew =
        List.filter(
          (Filter.{item, _}) =>
            matches(newQueryEx, format(item, ~shouldLower)),
          uiFiltered,
        );

      let allFilteredNew =
        List.filter(
          item => matches(newQueryEx, format(item, ~shouldLower)),
          allFiltered,
        );

      let newPendingWork = {
        ...p,
        filter: newQuery,
        explodedFilter: newQueryEx,
        shouldLower,
      };

      let newCompletedWork = {
        allFiltered: allFilteredNew,
        uiFiltered: uiFilteredNew,
      };

      (false, newPendingWork, newCompletedWork);
    } else {
      let newPendingWork = {
        ...p,
        filter: newQuery,
        explodedFilter: newQueryEx,
        itemsToFilter: p.allItems, // Reset items to filter
        shouldLower,
      };

      let newCompletedWork = initialCompletedWork;

      (false, newPendingWork, newCompletedWork);
    };
  };

  /* [addItems] is a helper for `Job.map` that updates the job when items have been added */
  let addItems = (items: list(Config.item), p: pendingWork, c: completedWork) => {
    let newPendingWork = {
      ...p,
      allItems: [items, ...p.allItems],
      totalItemCount: p.totalItemCount + List.length(items),
      itemsToFilter: [items, ...p.itemsToFilter],
    };

    (false, newPendingWork, c);
  };

  /* [doWork] is run each frame until the work is completed! */
  let doWork = (p: pendingWork, c: completedWork) => {
    let i = ref(0);
    let isCompleted = ref(false);
    let result = ref(None);

    let pendingWork = ref(p);
    let completedWork = ref(c.allFiltered);

    while (i^ < iterationsPerFrame && ! isCompleted^) {
      let p = pendingWork^;
      let c = completedWork^;
      let (newIsCompleted, newPendingWork, newCompletedWork) =
        switch (p.itemsToFilter) {
        | [] => (true, p, c)

        | [[], ...tail] => (false, {...p, itemsToFilter: tail}, c)

        | [[innerHd, ...innerTail], ...tail] =>
          // Do a first filter pass to check if the item satisifies the regex
          let name = format(innerHd, ~shouldLower=p.shouldLower);
          let newCompleted =
            if (matches(p.explodedFilter, name)) {
              [innerHd, ...c];
            } else {
              c;
            };

          (
            false,
            {...p, itemsToFilter: [innerTail, ...tail]},
            newCompleted,
          );
        };

      pendingWork := newPendingWork;
      completedWork := newCompletedWork;
      incr(i);
      isCompleted := newIsCompleted || isCompleted^;
      result := Some((newIsCompleted, newPendingWork, newCompletedWork));
    };

    switch (result^) {
    | None => (true, p, c)
    | Some((isCompleted, pendingWork, completedWork)) =>
      /* As a last pass, run the filter to sort / score filtered items if under a certain length */
      let uiFiltered =
        completedWork
        |> Utility.firstk(maxItemsToFilter)
        |> Filter.rank(p.filter, format);

      (isCompleted, pendingWork, {allFiltered: completedWork, uiFiltered});
    };
  };

  let progressReporter = (p: pendingWork, _) => {
    let toFilter = float_of_int(List.length(p.itemsToFilter));
    let total = float_of_int(List.length(p.allItems));

    1.0 -. toFilter /. total;
  };

  let create = () => {
    Job.create(
      ~pendingWorkPrinter=showPendingWork,
      ~completedWorkPrinter=showCompletedWork,
      ~progressReporter,
      ~name="FilterJob",
      ~initialCompletedWork,
      ~budget=Milliseconds(2.),
      ~f=doWork,
      initialPendingWork,
    );
  };

  let default = create();
};
