open EditorCoreTypes;
open Oni_Core;
open BenchFramework;

let createGiantString = iter => {
  let rec f = (s, i) => {
    i > 0
      ? f(s ++ "this is a really\tlong_linewithsomelongtokens", i - 1) : s;
  };

  f("", iter);
};
let makeLine = BufferLine.make(~measure=_ => 1.0);

let giantString = createGiantString(50) |> makeLine;

let splitEverything =
    (
      ~index0 as _,
      ~byte0 as _,
      ~char0 as _,
      ~index1 as _,
      ~byte1 as _,
      ~char1 as _,
    ) =>
  true;
let splitNothing =
    (
      ~index0 as _,
      ~byte0 as _,
      ~char0 as _,
      ~index1 as _,
      ~byte1 as _,
      ~char1 as _,
    ) =>
  false;

let tokenizeWithSplits = () => {
  let _: list(Tokenizer.TextRun.t) =
    Tokenizer.tokenize(
      ~f=splitEverything,
      ~stop=BufferLine.lengthSlow(giantString) |> CharacterIndex.ofInt,
      giantString,
    );
  ();
};

let tokenizeWithoutSplits = () => {
  let _: list(Tokenizer.TextRun.t) =
    Tokenizer.tokenize(
      ~f=splitNothing,
      ~stop=BufferLine.lengthSlow(giantString) |> CharacterIndex.ofInt,
      giantString,
    );
  ();
};

let tokenizeWithSplitsSub = () => {
  let _: list(Tokenizer.TextRun.t) =
    Tokenizer.tokenize(
      ~start=100 |> CharacterIndex.ofInt,
      ~stop=200 |> CharacterIndex.ofInt,
      ~f=splitEverything,
      giantString,
    );
  ();
};

let tokenizeWithoutSplitsSub = () => {
  let _: list(Tokenizer.TextRun.t) =
    Tokenizer.tokenize(
      ~start=100 |> CharacterIndex.ofInt,
      ~stop=200 |> CharacterIndex.ofInt,
      ~f=splitNothing,
      giantString,
    );
  ();
};

let options = Reperf.Options.create(~iterations=100, ());

let setup = () => ();

bench(
  ~name="Tokenizer: Tokenize large line entirely, high split count",
  ~options,
  ~setup,
  ~f=tokenizeWithSplits,
  (),
);

bench(
  ~name="Tokenizer: Tokenize large line entirely, no splits",
  ~options,
  ~setup,
  ~f=tokenizeWithoutSplits,
  (),
);

bench(
  ~name="Tokenizer: Tokenize large line subset (100-200), high split count",
  ~options,
  ~setup,
  ~f=tokenizeWithoutSplitsSub,
  (),
);

bench(
  ~name="Tokenizer: Tokenize large line subset (100-200), no splits",
  ~options,
  ~setup,
  ~f=tokenizeWithoutSplitsSub,
  (),
);
