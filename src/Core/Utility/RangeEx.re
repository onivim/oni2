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

let toByteLineMap: list(ByteRange.t) => IntMap.t(list(ByteRange.t)) =
  ranges => {
    List.fold_left(
      (prev, cur) =>
        ByteRange.(
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
