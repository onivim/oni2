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
 *   ranges([|1, 3, 4, 5, 7, 8|]) == [(1, 1), (3, 5), (7, 8)]
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

module Tests = {
  let%test "prevRollOver 3@0 == 3" = prevRollOver(~last=3, 0) == 3;
  let%test "prevRollOver 1..5@1 == 5" =
    prevRollOver(~first=1, ~last=5, 1) == 5;
  let%test "prevRollOver 1..5@0 == 5" =
    prevRollOver(~first=1, ~last=5, 0) == 5;
  let%test "prevRollOver 1..5@2 == 1" =
    prevRollOver(~first=1, ~last=5, 2) == 1;
  let%test "prevRollOver 1..5@5 == 4" =
    prevRollOver(~first=1, ~last=5, 5) == 4;
  let%test "prevRollOver 1..5@6 == 4" =
    prevRollOver(~first=1, ~last=5, 6) == 5;

  let%test "nextRollOver 3@0 == 3" = nextRollOver(~last=3, 3) == 0;
  let%test "nextRollOver 1..5@1 == 5" =
    nextRollOver(~first=1, ~last=5, 1) == 2;
  let%test "nextRollOver 1..5@0 == 5" =
    nextRollOver(~first=1, ~last=5, 0) == 1;
  let%test "nextRollOver 1..5@2 == 1" =
    nextRollOver(~first=1, ~last=5, 2) == 3;
  let%test "nextRollOver 1..5@5 == 4" =
    nextRollOver(~first=1, ~last=5, 5) == 1;
  let%test "nextRollOver 1..5@6 == 4" =
    nextRollOver(~first=1, ~last=5, 6) == 1;

  let%test "ranges" =
    ranges([|1, 3, 4, 5, 7, 8|]) == [(1, 1), (3, 5), (7, 8)];

  let%test "failure" = false;
};
