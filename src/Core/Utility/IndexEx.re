let prevRollOver = (~first=0, ~last, current) =>
  if (first >= last) {
    first;
  } else if (current <= first) {
    last;
  } else {
    current - 1;
  };

let prevRollOverOpt = (~first=0, ~last) =>
  fun
  | Some(index) => Some(prevRollOver(index, ~first, ~last))
  | None => Some(last);

let nextRollOver = (~first=0, ~last, current) =>
  if (first >= last) {
    first;
  } else if (current >= last) {
    first;
  } else {
    current + 1;
  };

let nextRollOverOpt = (~first=0, ~last) =>
  fun
  | Some(index) => Some(nextRollOver(index, ~first, ~last))
  | None => Some(first);

/**
 * Returns the list of tuples representing the ranges of consecutive numbers in the input array.
 *
 * E.g.
 *   ranges([|1, 3, 4, 5, 7, 8|]) == [(1, 2), (3, 5), (7, 8)]
 *
 * Assumes the array is sorted in increasing order
 */
let ranges = indices =>
  Array.fold_left(
    (acc, i) =>
      switch (acc) {
      | [] => [(i, i)]

      | [(low, high), ...rest] =>
        if (high + 1 == i) {
          [
            (low, i),
            ...rest // Extend current range
          ];
        } else {
          [
            (i, i),
            ...acc // Add new range
          ];
        }
      },
    [],
    indices,
  )
  |> List.rev;
