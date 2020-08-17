open Treesitter;
open BenchFramework;

Printexc.record_backtrace(true);

let jsonParser = Parser.json();
let cParser = Parser.c();

let simpleJson = "[1, \"2\", { \"test\": [1] }]";

let simpleC = "int main() { return 1; }";

let parse = (v: string, ()) => {
  let _ = Parser.parseString(jsonParser, v);
  ();
};

let parseArray = (v: array(string), ()) => {
  let _ = ArrayParser.parse(jsonParser, None, v);
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=10, ());

bench(
  ~name="parseString: Small JSON",
  ~options,
  ~setup,
  ~f=parse(simpleJson),
  (),
);

bench(~name="parseString: Small C", ~options, ~setup, ~f=parse(simpleC), ());

bench(
  ~name="parseString: Large JSON (canada.json)",
  ~options,
  ~setup,
  ~f=parse(TestData.largeJsonString),
  (),
);

bench(
  ~name="parseString: Large C (sqlite3.c)",
  ~options,
  ~setup,
  ~f=parse(TestData.largeCString),
  (),
);

bench(
  ~name="parseArray: Large JSON (canada.json)",
  ~options,
  ~setup,
  ~f=parseArray(TestData.largeJsonArray),
  (),
);

bench(
  ~name="parseArray: Large C (sqlite3.c)",
  ~options,
  ~setup,
  ~f=parseArray(TestData.largeCArray),
  (),
);
