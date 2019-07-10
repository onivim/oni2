open Oni_Core;
open BenchFramework;

let createGiantString = iter => {
  let rec f = (s, i) => {
    i > 0
      ? f(s ++ "this is a really\tlong_linewithsomelongtokens", i - 1) : s;
  };

  f("", iter);
};

let giantString = createGiantString(50);

let splitEverything = (_, _, _, _) => true;
let splitNothing = (_, _, _, _) => false;

let tokenizeWithSplits = () => {
  let _ = Tokenizer.tokenize(~f=splitEverything, giantString);
  ();
};

let tokenizeWithoutSplits = () => {
  let _ = Tokenizer.tokenize(~f=splitNothing, giantString);
  ();
};

let tokenizeWithSplitsSub = () => {
  let _ =
    Tokenizer.tokenize(
      ~startIndex=100,
      ~endIndex=200,
      ~f=splitEverything,
      giantString,
    );
  ();
};

let tokenizeWithoutSplitsSub = () => {
  let _ =
    Tokenizer.tokenize(
      ~startIndex=100,
      ~endIndex=200,
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
