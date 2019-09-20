/*
 * IntMap.re
 *
 * Map from int -> 'a
 */

include Map.Make({
  type t = int;
  let compare = compare;
});

/*
 * Shift adjust the locations of the map by a specified _delta_
 *
 * This is helpful for line-based consumers of the map - often, when lines are added / removed,
 * we want to shift existing lines to line up with the new delta.
 */
let shift = (map: t('a), startPos: int, endPos: int, delta: int) =>
  if (endPos - startPos == delta) {
    map;
  } else {
      fold(
        (key, v, prev) =>
          if (delta > 0) {
            if (key < startPos) {
              update(key, _opt => Some(v), prev);
            } else {
              update(key + delta, _opt => Some(v), prev);
            };
          } else if (key <= endPos) {
            update(key, _opt => Some(v), prev);
          } else {
            update(key + delta, _opt => Some(v), prev);
          },
        map,
        empty,
      );
  };
