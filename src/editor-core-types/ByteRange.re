[@deriving show]
type t = {
  start: BytePosition.t,
  stop: BytePosition.t,
};

let zero = {start: BytePosition.zero, stop: BytePosition.zero};

let contains = (position: BytePosition.t, range) => {
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
