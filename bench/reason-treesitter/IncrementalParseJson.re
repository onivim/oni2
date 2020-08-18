open Treesitter;
open BenchFramework;

Printexc.record_backtrace(true);

let jsonParser = Parser.json();

let simpleJson = "[1, \"2\", { \"test\": [1] }]";

let (_, baseline) =
  ArrayParser.parse(jsonParser, None, TestData.largeJsonArray);

let createDelta = () => {
  let _ = ArrayParser.Delta.create(baseline, 5, 6, [|"},", "2,", "3,"|]);
  ();
};

let delta = ArrayParser.Delta.create(baseline, 5, 6, [|"},", "2,", "3,"|]);

let reparse = () => {
  let _ =
    ArrayParser.parse(jsonParser, Some(delta), TestData.largeJsonArray);
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=10, ());

bench(
  ~name="[Incremental] JSON -  ArrayParser.Delta.create",
  ~options,
  ~setup,
  ~f=createDelta,
  (),
);

bench(
  ~name="[Incremental] JSON -  ArrayParser.parse w/ delta update",
  ~options,
  ~setup,
  ~f=reparse,
  (),
);
