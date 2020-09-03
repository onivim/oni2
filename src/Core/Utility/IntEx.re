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

let modulo = (a, b) => {
  // https://stackoverflow.com/questions/46758683/ocaml-mod-function-returns-different-result-compared-with
  let result = a mod b;
  if (result >= 0) {
    result;
  } else {
    result + b;
  };
};

/**
 * Returns `n` bounded by `hi` and `lo`, wrapping at boundaries
 *
 * Assumes `hi` is larger than `lo`
 */
let wrap = (n, ~hi, ~lo) => modulo(n - lo, hi - lo + 1) + lo;

let%test "wrap 1..3@1" = wrap(~lo=1, ~hi=3, 1) == 1;
let%test "wrap 1..3@4" = wrap(~lo=1, ~hi=3, 4) == 1;
let%test "wrap 1..3@3" = wrap(~lo=1, ~hi=3, 3) == 3;
let%test "wrap 1..3@0" = wrap(~lo=1, ~hi=3, 0) == 3;
let%test "wrap 1..3@0" = wrap(~lo=1, ~hi=3, -1) == 2;
let%test "wrap 1..1@0" = wrap(~lo=1, ~hi=1, -1) == 1;
let%test "wrap 1..1@0" = wrap(~lo=1, ~hi=1, 2) == 1;
