/*
   BufferLineJob.re

   BufferLineJob is a Job.t that breaks up work that happens across buffer lines.
 */

open Oni_Core;
open Oni_Core.Types;

type pendingWork('context) = {
  // List of ranges that are visible
  visibleRanges: list(list(Range.t)),
  // Processed ranges - list of ranges we need to go through
  remainingRanges: list(list(Range.t)),
  // The latest buffer version that came our away
  version: int,
  // Context for to handle a variety of 'per-line' cases.
  // The consumer would provide some context - for example, for tree-sitter,
  // we'd provide the latest Tree and lines.
  context: 'context,
};

type lineInfo('v) = {
  // The last buffer version we evaluated this line at
  // If the version is newer, we might need to look at it again!
  version: int,
  // v is the actual completed work from the job
  v: 'v,
};

type completedWork('v) = {lines: IntMap.t(lineInfo('v))};

type t('context, 'v) = Job.t(pendingWork('context), completedWork('v));

let getContext = (v: t('context, 'v)) => {
  Job.getPendingWork(v).context;
};

let getCompletedWork = (line: int, v: t('context, 'v)) => {
  let cw = Job.getCompletedWork(v);

  switch (IntMap.find_opt(line, cw.lines)) {
  | None => None
  | Some(lineInfo) => Some(lineInfo.v)
  };
};

let clear = (~newContext=None, v: t('context, 'v)) => {
  let oldContext = getContext(v);
  let context =
    switch (newContext) {
    | Some(c) => c
    | None => oldContext
    };

  let f = (p, _c) => {
    let newPendingWork = {...p, remainingRanges: p.visibleRanges, context};

    let newCompletedWork = {lines: IntMap.empty};

    (false, newPendingWork, newCompletedWork);
  };
  Job.map(f, v);
};

let showPendingWork = (~contextPrinter=_ => "n/a", v: pendingWork('context)) => {
  "- Pending Work\n"
  ++ " -- latest buffer version: "
  ++ string_of_int(v.version)
  ++ " -- ranges left to process: "
  ++ string_of_int(List.length(v.remainingRanges))
  ++ " -- context: "
  ++ contextPrinter(v.context);
};

let showCompletedWork = (~workPrinter=_ => "n/a", v: completedWork('v)) => {
  List.fold_left(
    (prev, curr) => {
      let (key, info) = curr;
      prev
      ++ "\n"
      ++ "Line "
      ++ string_of_int(key)
      ++ ": "
      ++ " version "
      ++ string_of_int(info.version)
      ++ " - work: "
      ++ workPrinter(info.v);
    },
    "",
    IntMap.bindings(v.lines),
  );
};

let initialCompletedWork = {lines: IntMap.empty};

let initialPendingWork = context => {
  visibleRanges: [],
  remainingRanges: [],
  version: (-1),
  context,
};

let updateContext = (context: 'context, v: t('context, 'v)) => {
  let f = (pending, completed) => {
    (false, {...pending, context}, completed);
  };

  Job.map(f, v);
};

let notifyBufferUpdate = (version: int, v: t('context, 'v)) => {
  let f = (pending, completed) => {
    (
      false,
      {
        ...pending,
        version,
        // When the version changes, we'll go through visible lines and see what we need todo
        remainingRanges: pending.visibleRanges,
      },
      completed,
    );
  };

  Job.map(f, v);
};

let setVisibleRanges = (newRanges: list(list(Range.t)), v: t('context, 'v)) => {
  let f = (pending, completed) => {
    (
      false,
      {...pending, visibleRanges: newRanges, remainingRanges: newRanges},
      completed,
    );
  };

  Job.map(f, v);
};

let doWork = (f, p: pendingWork('context), c: completedWork('v)) => {
  switch (p.remainingRanges) {
  // If no processed ranges left, we're done!
  | [] => (true, p, c)
  | [outerHd, ...outerTail] =>
    switch (outerHd) {
    | [] => (false, {...p, remainingRanges: outerTail}, c)
    | [hd, ...tail] =>
      let range: Range.t = hd;
      let line = range.startPosition.line |> Index.toInt0;

      let doLineWork = () => {
        f(p.context, line);
      };

      let lines =
        IntMap.update(
          line,
          prev =>
            switch (prev) {
            | Some(v) =>
              if (v.version >= p.version) {
                // If this is up-to-date, just pass it through
                Some(v);
              } else {
                Some({version: p.version, v: doLineWork()});
              }
            | None => Some({version: p.version, v: doLineWork()})
            },
          c.lines,
        );

      let remainingRanges =
        switch (tail) {
        | [] => outerTail
        | t => [t, ...outerTail]
        };

      let newPendingWork = {...p, remainingRanges};

      let newCompletedWork = {lines: lines};

      (false, newPendingWork, newCompletedWork);
    }
  };
};

let create = (~name, ~initialContext, ~f) => {
  Job.create(
    ~pendingWorkPrinter=showPendingWork,
    ~completedWorkPrinter=showCompletedWork,
    ~name,
    ~initialCompletedWork,
    ~budget=Milliseconds(2.),
    ~f=doWork(f),
    initialPendingWork(initialContext),
  );
};
