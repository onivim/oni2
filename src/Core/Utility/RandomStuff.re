type commandLineCompletionMeet = {
  prefix: string,
  position: int,
};

let getCommandLineCompletionsMeet = (str: string, position: int) => {
  let len = String.length(str);

  if (len == 0 || position < len) {
    None;
  } else {
    /* Look backwards for '/' or ' ' */
    let found = ref(false);
    let meet = ref(position);

    while (meet^ > 0 && ! found^) {
      let pos = meet^ - 1;
      let c = str.[pos];
      if (c == ' ') {
        found := true;
      } else {
        decr(meet);
      };
    };

    let pos = meet^;
    Some({prefix: String.sub(str, pos, len - pos), position: pos});
  };
};

let executingDirectory = Revery.Environment.executingDirectory;

external freeConsole: unit => unit = "win32_free_console";

/**
 * Returns `n` bounded by `hi` and `lo`
 *
 * E.g.
 *   clamp(0, ~hi=1, ~lo=-1) == 0
 *   clamp(-1, ~hi=1, ~lo=0) == 0
 *   clamp(1, ~hi=0, ~lo=-1) == 0
 *
 * Assumes `hi` is larger than `lo`
 */
let clamp = (n, ~hi, ~lo) => max(lo, min(hi, n));

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

let tap = (f, x) => {
  f(x);
  x;
};
