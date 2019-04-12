open Oni_Core;
open BenchFramework;

let giantString = String.make(1000, 'a');

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

let options = Reperf.Options.create(~iterations=1000, ());

let setup = () => ();

bench(
  ~name="Tokenizer: Tokenize large line, high split count",
  ~options,
  ~setup,
  ~f=tokenizeWithSplits,
  (),
);

bench(
  ~name="Tokenizer: Tokenize large line, no splits",
  ~options,
  ~setup,
  ~f=tokenizeWithoutSplits,
  (),
);
