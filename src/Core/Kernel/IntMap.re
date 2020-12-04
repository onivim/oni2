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
    // Shift all items based on delta
    let newMap =
      map
      |> bindings
      |> List.map(((key, v)) =>
           if (delta > 0) {
             if (key < startPos) {
               (key, v);
             } else {
               (key + delta, v);
             };
           } else if (key <= endPos) {
             (key, v);
           } else {
             (key + delta, v);
           }
         )
      |> List.to_seq
      |> of_seq;

    // Set 'new' items to be the default value
    let current = find_opt(startPos, newMap);
    let stop = startPos + (endPos - startPos + delta);
    let ret = ref(newMap);

    for (idx in startPos to stop - 1) {
      ret := update(idx, _ => default(current), ret^);
    };

    ret^;
  };
