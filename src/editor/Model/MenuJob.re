/* MenuJob.re

     MenuJob is a Job.t that describes how to break up the work of filtering
     menu items across multiple frames.
   */

open Oni_Core;

open CamomileBundled.Camomile;
module Zed_utf8 = Oni_Core.ZedBundled;

type pendingWork = {
  filter: string,
  explodedFilter: list(UChar.t),
  // Full commands is the _complete set_ of unfiltered commands
  // This never gets filtered - it's persisted in case we need
  // the full set again
  fullCommands: list(Actions.menuCommand),
  // Commands to filter are commands we haven't looked at yet.
  commandsToFilter: list(Actions.menuCommand),
};

let showPendingWork = (v: pendingWork) => {
  "- Pending Work\n"
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
  explodedFilter: [],
  fullCommands: [],
  commandsToFilter: [],
};

// Constants
let iterationsPerFrame = 250;
let maxItemsToFilter = 1000;

/* [addItems] is a helper for `Job.map` that updates the job when the query has changed */
let updateQuery = (newQuery: string, p: pendingWork, _c: completedWork) => {
  // TODO: Optimize - for now, if the query changes, just clear the completed work
  // However, there are several ways we could improve this:
  // - If the query is just a stricter version... we could add the filter items back to completed
  // - If the query is broader, we could keep our current filtered items anyway

  let newPendingWork = {
    ...p,
    filter: newQuery,
    explodedFilter: Zed_utf8.explode(newQuery),
    commandsToFilter: p.fullCommands // Reset the commands to filter
  };

  let newCompletedWork = initialCompletedWork;

  (false, newPendingWork, newCompletedWork);
};

let matches = (query: list(UChar.t), str) => {
  let toMatch = Zed_utf8.explode(str);

  let rec f = (q, m) => {
    switch (q, m) {
    | ([], _) => true
    | (_, []) => false
    | ([qh, ...qtail], [mh, ...mtail]) when UChar.eq(qh, mh) =>
      f(qtail, mtail)
    | (q, [mh, ...mtail]) => f(q, mtail)
    };
  };

  f(query, toMatch);
};

/* [addItems] is a helper for `Job.map` that updates the job when items have been added */
let addItems =
    (items: list(Actions.menuCommand), p: pendingWork, c: completedWork) => {
  let newPendingWork = {
    ...p,
    fullCommands: List.append(items, p.fullCommands),
    commandsToFilter: List.append(items, p.commandsToFilter),
  };

  (false, newPendingWork, c);
};

let getStringToTest = (v: Actions.menuCommand) =>
  switch (v.category) {
  | Some(c) => c ++ v.name
  | None => v.name
  };

/* [doWork] is run each frame until the work is completed! */
let doWork = (p: pendingWork, c: completedWork) => {
  let i = ref(0);
  let completed = ref(false);
  let result = ref(None);

  let pendingWork = ref(p);
  let completedWork = ref(c.allFiltered);

  while (i^ < iterationsPerFrame && ! completed^) {
    let p = pendingWork^;
    let c = completedWork^;
    let (c, newPendingWork, newCompletedWork) =
      switch (p.commandsToFilter) {
      | [] => (true, p, c)
      | [hd, ...tail] =>
        let newCompleted =
          matches(p.explodedFilter, getStringToTest(hd)) ? [hd, ...c] : c;
        (false, {...p, commandsToFilter: tail}, newCompleted);
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
      |> Filter.menu(p.filter)
      |> Array.of_list;
    (completed, p, {allFiltered: c, uiFiltered});
  };
};

let create = () => {
  Job.create(
    ~pendingWorkPrinter=showPendingWork,
    ~completedWorkPrinter=showCompletedWork,
    ~name="MenuJob",
    ~initialCompletedWork,
    ~f=doWork,
    initialPendingWork,
  );
};

let default = create();
