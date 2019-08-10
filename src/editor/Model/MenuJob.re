/* MenuJob.re

     MenuJob is a Job.t that describes how to break up the work of filtering
     menu items across multiple frames.
   */

open Oni_Core;

type pendingWork = {
  filter: string,
  regex: Str.regexp,
  // Full commands is the _complete set_ of unfiltered commands
  // This never gets filtered - it's persisted in case we need
  // the full set again
  fullCommands: list(Actions.menuCommand),
  // Commands to filter are commands we haven't looked at yet.
  commandsToFilter: list(Actions.menuCommand),
};

type completedWork = list(Actions.menuCommand);
type t = Job.t(pendingWork, completedWork);

let initialCompletedWork = [];
let initialPendingWork = {
  filter: "",
  regex: Str.regexp(".*"),
  fullCommands: [],
  commandsToFilter: [],
};

let iterationsPerFrame = 5000;

// TODO: abc -> .*a.*b.*c
//let regexFromFilter = s => Str.regexp(".*");

let regexFromFilter = s => {
    let a = s
    |> String.to_seq
    |> Seq.map((c) => String.make(1, c))
    |> List.of_seq;
    let b = String.concat(".*", a);
    let c = ".*" ++ b ++ ".*";
    Str.regexp(c);
 };

/* [addItems] is a helper for `Job.map` that updates the job when the query has changed */
let updateQuery = (newQuery: string, p: pendingWork, _c: completedWork) => {
  // TODO: Optimize - for now, if the query changes, just clear the completed work
  // However, there are several ways we could improve this:
  // - If the query is just a stricter version... we could add the filter items back to completed
  // - If the query is broader, we could keep our current filtered items anyway

  let newPendingWork = {
    ...p,
    filter: newQuery,
    regex: regexFromFilter(newQuery),
    commandsToFilter: p.fullCommands // Reset the commands to filter
  };

  let newCompletedWork = [];

  (false, newPendingWork, newCompletedWork);
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

let getStringToTest = (v: Actions.menuCommand) => switch (v.category) {
| Some(c) => c ++ v.name
| None => v.name
};

/* [doWork] is run each frame until the work is completed! */
let doWork = (p: pendingWork, c: completedWork) => {
  let i = ref(0);
  let completed = ref(false);
  let result = ref(None);

  while (i^ < iterationsPerFrame && ! completed^) {
    let (completed, newPendingWork, newCompletedWork) =
      switch (p.commandsToFilter) {
      | [] => (true, p, c)
      | [hd, ...tail] =>
        // Do a first filter pass to check if the item satisifies the regex
        let newCompleted = Str.string_match(p.regex, getStringToTest(hd), 0) ? [hd, ...c] : c;
        (false, {...p, commandsToFilter: tail}, newCompleted);
      };
    incr(i);

    result := Some((completed, newPendingWork, newCompletedWork));
  };

  switch (result^) {
  | None => (true, p, c)
  | Some((completed, p, c)) => (completed, p, c)
  };
};

let create = () => {
  Job.create(
    ~name="MenuJob",
    ~initialCompletedWork=[],
    ~f=doWork,
    initialPendingWork,
  );
};

let default = create();
