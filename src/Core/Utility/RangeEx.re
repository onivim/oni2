open EditorCoreTypes;
open Kernel;

let toLineMap: list(Range.t) => IntMap.t(list(Range.t)) =
  ranges => {
    List.fold_left(
      (prev, cur) =>
        Range.(
          IntMap.update(
            Index.toZeroBased(cur.start.line),
            v =>
              switch (v) {
              | None => Some([cur])
              | Some(v) => Some([cur, ...v])
              },
            prev,
          )
        ),
      IntMap.empty,
      ranges,
    );
  };

let areRangesEqual: (list(Range.t), list(Range.t)) => bool =
  (rangesA, rangesB) => {
    let rec loop = (rA, rB) => {
      switch (rA, rB) {
      | ([], []) => true
      | ([hdA, ...tailA], [hdB, ...tailB]) =>
        hdA == hdB && loop(tailA, tailB)
      | ([], _) => false
      | (_, []) => false
      };
    };

    loop(rangesA, rangesB);
  };
