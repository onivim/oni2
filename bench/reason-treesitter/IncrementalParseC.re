open Treesitter;
open BenchFramework;

Printexc.record_backtrace(true);

let cParser = Parser.c();

let (_, baseline) = ArrayParser.parse(cParser, None, TestData.largeCArray);

let createDelta = () => {
  let _ =
    ArrayParser.Delta.create(
      baseline,
      190279,
      190280,
      [|"#define A", "#define B"|],
    );
  ();
};

let delta =
  ArrayParser.Delta.create(
    baseline,
    190279,
    190280,
    [|"#define A", "#define B"|],
  );

let reparse = () => {
  let _ = ArrayParser.parse(cParser, Some(delta), TestData.largeCArray);
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=10, ());

bench(
  ~name="[Incremental] C - ArrayParser.Delta.create",
  ~options,
  ~setup,
  ~f=createDelta,
  (),
);

bench(
  ~name="[Incremental] C - ArrayParser.parse w/ delta update",
  ~options,
  ~setup,
  ~f=reparse,
  (),
);
