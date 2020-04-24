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
let noop = _ => None;
let shift =
    (
      ~default: option('a) => option('a)=noop,
      ~startPos: int,
      ~endPos: int,
      ~delta: int,
      map,
    ) =>
  if (delta == 0) {
    map;
  } else {
    let original: t('a) = map;
    // Shift all items based on delta
    let newMap =
      fold(
        (key, oldValue, prev) =>
          if (delta > 0) {
            if (key < startPos) {
              prev;
            } else {
              update(key + delta, _ => Some(oldValue), prev);
            };
          } else if (key <= endPos) {
            prev;
          } else {
            let originalValue: option('a) = find_opt(key, original);
            update(key + delta, _ => originalValue, prev);
          },
        map, /* map to fold over */
        map /* seed defaults */
      );

    // Set 'new' items to be the default value
    let current = find_opt(startPos, newMap);
    let stop = startPos + (endPos - startPos + delta);
    let ret = ref(newMap);

    for (idx in startPos to stop - 1) {
      ret := update(idx, _ => default(current), ret^);
    };

    ret^;
  };
