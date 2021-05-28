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

let toByteLineMap: list(ByteRange.t) => IntMap.t(list(ByteSpan.t)) =
  ranges => {
    List.fold_left(
      (prev: IntMap.t(list(ByteSpan.t)), cur: ByteRange.t) => {
        let maybeSpan = ByteSpan.ofRange(cur);
        if (maybeSpan == None) {
          prev;
        } else {
          let span = maybeSpan |> Option.get;
          IntMap.update(
            EditorCoreTypes.LineNumber.toZeroBased(cur.start.line),
            v =>
              switch (v) {
              | None => Some([span])
              | Some(v) => Some([span, ...v])
              },
            prev,
          );
        };
      },
      IntMap.empty,
      ranges,
    );
  };

let toCharacterLineMap:
  list(CharacterRange.t) => IntMap.t(list(CharacterRange.t)) =
  ranges => {
    List.fold_left(
      (prev, cur) =>
        CharacterRange.(
          IntMap.update(
            EditorCoreTypes.LineNumber.toZeroBased(cur.start.line),
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
