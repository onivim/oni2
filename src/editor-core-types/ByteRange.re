[@deriving show]
type t = {
  start: BytePosition.t,
  stop: BytePosition.t,
};

let zero = {start: BytePosition.zero, stop: BytePosition.zero};

module Internal = {
  let normalizeLines = range =>
    if (range.start.line > range.stop.line) {
      {start: range.stop, stop: range.start};
    } else {
      range;
    };

  let normalizeByte = range =>
    if (range.start.line == range.stop.line
        && ByteIndex.toInt(range.stop.byte)
        < ByteIndex.toInt(range.start.byte)) {
      {start: range.stop, stop: range.start};
    } else {
      range;
    };
};

let compare = (a, b) => {
  // Compare start positions, unless they are equal - then use end positions
  let (aPosition, bPosition) =
    BytePosition.equals(a.start, b.start)
      ? (a.stop, b.stop) : (a.start, b.start);

  BytePosition.compare(aPosition, bPosition);
};

let normalize = range =>
  range |> Internal.normalizeLines |> Internal.normalizeByte;

let contains = (position: BytePosition.t, range) => {
  let range = normalize(range);
  (
    LineNumber.(position.line == range.start.line)
    && ByteIndex.(position.byte >= range.start.byte)
    || LineNumber.(position.line > range.start.line)
  )
  && (
    LineNumber.(position.line == range.stop.line)
    && ByteIndex.(position.byte <= range.stop.byte)
    || LineNumber.(position.line < range.stop.line)
  );
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

let equals = (a, b) => BytePosition.(a.start == b.start && a.stop == b.stop);
