/* MenuJob.re

     MenuJob is a Job.t that describes how to break up the work of filtering
     menu items across multiple frames.
   */

open Oni_Core;

open CamomileBundled.Camomile;
module Zed_utf8 = Oni_Core.ZedBundled;


let format = (item, ~shouldLower) => {
  let s = Menu.getLabel(item);
  shouldLower ? String.lowercase_ascii(s) : s;
};

type pendingWork = {
  filter: string,
  // Full commands is the _complete set_ of unfiltered commands
  // This never gets filtered - it's persisted in case we need
  // the full set again
  explodedFilter: list(UChar.t),
  shouldLower: bool,
  totalCommandCount: int,
  fullCommands: list(list(Actions.menuCommand)),
  // Commands to filter are commands we haven't looked at yet.
  commandsToFilter: list(list(Actions.menuCommand)),
};

let showPendingWork = (v: pendingWork) => {
  "- Pending Work\n"
  ++ " -- totalCommandCount: "
  ++ string_of_int(v.totalCommandCount)
  ++ " -- fullCommands: "
  ++ string_of_int(List.length(v.fullCommands))
  ++ " -- commandsToFilter: "
  ++ string_of_int(List.length(v.commandsToFilter));
};

type completedWork = {
  allFiltered: list(Actions.menuCommand),
  // If the allFiltered list is still huge,
  // we take a subset prior to sorting to display in the UI
  // The 'ui' filtered should be the main item for the UI to use
  uiFiltered: array(Actions.menuCommand),
};

let showCompletedWork = (v: completedWork) => {
  "- Completed Work\n"
  ++ " -- allFiltered: "
  ++ string_of_int(List.length(v.allFiltered))
  ++ " -- uiFiltered: "
  ++ string_of_int(Array.length(v.uiFiltered));
};

type t = Job.t(pendingWork, completedWork);

let initialCompletedWork = {allFiltered: [], uiFiltered: [||]};
let initialPendingWork = {
  filter: "",
  fullCommands: [],
  explodedFilter: [],
  shouldLower: false,
  commandsToFilter: [],
  totalCommandCount: 0,
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

    let uiFilteredList = Array.to_list(uiFiltered);
    let uiFilteredNew =
      List.filter(
        item => matches(newQueryEx, format(item, ~shouldLower)),
        uiFilteredList,
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
      uiFiltered: Array.of_list(uiFilteredNew),
    };

    (false, newPendingWork, newCompletedWork);
  } else {
    let newPendingWork = {
      ...p,
      filter: newQuery,
      explodedFilter: newQueryEx,
      commandsToFilter: p.fullCommands // Reset the commands to filter
    };

    let newCompletedWork = initialCompletedWork;

    (false, newPendingWork, newCompletedWork);
  };
};

/* [addItems] is a helper for `Job.map` that updates the job when items have been added */
let addItems =
    (items: list(Actions.menuCommand), p: pendingWork, c: completedWork) => {
  let newPendingWork = {
    ...p,
    fullCommands: [items, ...p.fullCommands],
    totalCommandCount: p.totalCommandCount + List.length(items),
    commandsToFilter: [items, ...p.commandsToFilter],
  };

  (false, newPendingWork, c);
};

/* [doWork] is run each frame until the work is completed! */
let doWork = (p: pendingWork, c: completedWork) => {
  let i = ref(0);
  let completed = ref(false);
  let result = ref(None);

  let pendingWork = ref(p);
  let completedWork = ref(c.allFiltered);

  while (i^ < iterationsPerFrame && ! completed^) {
    let {shouldLower, _} as p = pendingWork^;
    let c = completedWork^;
    let (c, newPendingWork, newCompletedWork) =
      switch (p.commandsToFilter) {
      | [] => (true, p, c)
      | [hd, ...tail] =>
        switch (hd) {
        | [] => (false, {...p, commandsToFilter: tail}, c)
        | [innerHd, ...innerTail] =>
          // Do a first filter pass to check if the item satisifies the regex
          let newCompleted =
            matches(p.explodedFilter, format(innerHd, ~shouldLower))
              ? [innerHd, ...c] : c;
          (
            false,
            {...p, commandsToFilter: [innerTail, ...tail]},
            newCompleted,
          );
        }
      };
    pendingWork := newPendingWork;
    completedWork := newCompletedWork;
    incr(i);
    if (c) {
      completed := c;
    };
    result := Some((c, newPendingWork, newCompletedWork));
  };

  switch (result^) {
  | None => (true, p, c)
  | Some((completed, p, c)) =>
    /* As a last pass, run the menu filter to sort / score filtered items if under a certain length */
    let uiFiltered =
      c
      |> Utility.firstk(maxItemsToFilter)
      |> Filter.rank(p.filter, format)
      |> List.map((Filter.{item, highlight}) =>
           Actions.{...item, highlight}
         )
      |> Array.of_list;
    (completed, p, {allFiltered: c, uiFiltered});
  };
};

let progressReporter = (p: pendingWork, _) => {
  1.0
  -. float_of_int(List.length(p.commandsToFilter))
  /. float_of_int(List.length(p.fullCommands));
};

let create = () => {
  Job.create(
    ~pendingWorkPrinter=showPendingWork,
    ~completedWorkPrinter=showCompletedWork,
    ~progressReporter,
    ~name="MenuJob",
    ~initialCompletedWork,
    ~budget=Milliseconds(2.),
    ~f=doWork,
    initialPendingWork,
  );
};

let default = create();
