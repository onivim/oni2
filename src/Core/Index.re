[@deriving show({with_path: false})]
type t =
  | ZeroBasedIndex(int)
  | OneBasedIndex(int);

let toZeroBasedInt = (pos: t) =>
  switch (pos) {
  | ZeroBasedIndex(n) => n
  | OneBasedIndex(n) => n - 1
  };

let toInt0 = toZeroBasedInt;

let toOneBasedInt = (pos: t) =>
  switch (pos) {
  | ZeroBasedIndex(n) => n + 1
  | OneBasedIndex(n) => n
  };

let toInt1 = toOneBasedInt;

let ofInt0 = i => ZeroBasedIndex(i);
let ofInt1 = i => OneBasedIndex(i);

let equals = (a: t, b: t) => toInt0(a) == toInt0(b);
