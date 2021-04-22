[@deriving show({with_path: false})]
type t = {
  start: Location.t,
  stop: Location.t,
};

let create = (~start, ~stop) => {start, stop};

let explode = (measure, range) =>
  if (range.start.line == range.stop.line) {
    [range];
  } else {
    let ranges = ref([]);
    let pushRange = (~start, ~stop) =>
      ranges := [create(~start, ~stop), ...ranges^];

    let i = ref(range.start.line);
    while (i^ < range.stop.line) {
      let startColumn =
        i^ == range.start.line ? range.start.column : Index.zero;
      let stopColumn = measure(i^) |> max(1) |> Index.fromOneBased;

      pushRange(
        ~start=Location.create(~line=i^, ~column=startColumn),
        ~stop=Location.create(~line=i^, ~column=stopColumn),
      );

      i := Index.(i^ + 1);
    };

    pushRange(
      ~start=Location.create(~line=range.stop.line, ~column=Index.zero),
      ~stop=Location.create(~line=range.stop.line, ~column=range.stop.column),
    );

    ranges^ |> List.rev;
  };

let toHash = ranges => {
  let hash = Hashtbl.create(100);

  List.iter(
    range => {
      let line = range.start.line;
      let lineRanges =
        switch (Hashtbl.find_opt(hash, line)) {
        | Some(ranges) => ranges
        | None => []
        };

      Hashtbl.add(hash, line, [range, ...lineRanges]);
    },
    ranges,
  );

  hash;
};

let minLine = ranges => {
  let locationToLine = (location: Location.t) => {
    location.line |> Index.toZeroBased |> LineNumber.ofZeroBased;
  };

  let minOr = (location: Location.t, maybeCandidate) => {
    let candidate = location |> locationToLine;
    switch (maybeCandidate) {
    | None => Some(candidate)
    | Some(existing) as cur =>
      if (LineNumber.(candidate < existing)) {
        Some(candidate);
      } else {
        cur;
      }
    };
  };

  ranges
  |> List.fold_left(
       (maybeMin, range) => {
         maybeMin |> minOr(range.start) |> minOr(range.stop)
       },
       None,
     );
};

let maxLine = ranges => {
  let locationToLine = (location: Location.t) => {
    location.line |> Index.toZeroBased |> LineNumber.ofZeroBased;
  };

  let maxOr = (location: Location.t, maybeCandidate) => {
    let candidate = location |> locationToLine;
    switch (maybeCandidate) {
    | None => Some(candidate)
    | Some(existing) as cur =>
      if (LineNumber.(candidate > existing)) {
        Some(candidate);
      } else {
        cur;
      }
    };
  };

  ranges
  |> List.fold_left(
       (maybeMin, range) => {
         maybeMin |> maxOr(range.start) |> maxOr(range.stop)
       },
       None,
     );
};

let equals = (a, b) => Location.(a.start == b.start && a.stop == b.stop);

let contains = (position: Location.t, range) => {
  (
    position.line == range.start.line
    && position.column >= range.start.column
    || position.line > range.start.line
  )
  && (
    position.line == range.stop.line
    && position.column <= range.stop.column
    || position.line < range.stop.line
  );
};

let toString = ({start, stop}) =>
  Printf.sprintf(
    "Range - start: %s end: %s",
    Location.toString(start),
    Location.toString(stop),
  );
