open Textmate;
open BenchFramework;

let reasonJson = Yojson.Safe.from_file("test/reason-textmate/onivim/fixtures/reason.json");
let javascriptJson =
  Yojson.Safe.from_file("test/reason-textmate/first-mate/fixtures/javascript.json");
let cssJson = Yojson.Safe.from_file("test/reason-textmate/first-mate/fixtures/css.json");

let getGrammar = json =>
  switch (Grammar.Json.of_yojson(json)) {
  | Ok(v) => v
  | Error(msg) => failwith("Unable to load grammar: " ++ msg)
  };

let reasonGrammar = getGrammar(reasonJson);
let javascriptGrammar = getGrammar(javascriptJson);
let cssGrammar = getGrammar(cssJson);

let reasonGrammarRepository =
  GrammarRepository.ofGrammar("source.reason", reasonGrammar);
let javascriptGrammarRepository =
  GrammarRepository.ofGrammar("source.js", javascriptGrammar);

let cssGrammarRepository =
  GrammarRepository.ofGrammar("source.css", cssGrammar);

let read_file = filename => {
  let lines = ref([]);
  let chan = open_in(filename);
  try(
    {
      while (true) {
        lines := [input_line(chan), ...lines^];
      };
      lines^;
    }
  ) {
  | End_of_file =>
    close_in(chan);
    List.rev(lines^);
  };
};

let largeJs = Array.of_list(read_file("bench/collateral/large.js"));
let largeCss = Array.of_list(read_file("bench/collateral/bootstrap.css"));

let tokenizeFile = (grammarRepo, scope, lines, ()) => {
  let len = Array.length(lines);
  let idx = ref(0);
  let scopeStack = ref(None);
  let tokenizer = Tokenizer.create(~repository=grammarRepo, ());

  while (idx^ < len) {
    let line = lines[idx^] ++ "\n";
    let (_, newScopeStack) =
      Tokenizer.tokenize(
        ~lineNumber=idx^,
        ~scopeStack=scopeStack^,
        ~scope,
        tokenizer,
        line,
      );

    scopeStack := Some(newScopeStack);
    incr(idx);
  };
};

let simpleTokenization = () => {
  let tokenizer = Tokenizer.create(~repository=reasonGrammarRepository, ());
  let _ =
    Tokenizer.tokenize(
      ~lineNumber=0,
      ~scopeStack=None,
      ~scope="source.reason",
      tokenizer,
      "let add = (a, b) => a + b;",
    );
  ();
};

let setup = () => ();
let options = Reperf.Options.create(~iterations=10000, ());
let singleOption = Reperf.Options.create(~iterations=1, ());

bench(
  ~name="tokenize: simple case (reason)",
  ~options,
  ~setup,
  ~f=simpleTokenization,
  (),
);

bench(
  ~name="tokenize: Large JS file (jQuery)",
  ~options=singleOption,
  ~setup,
  ~f=tokenizeFile(javascriptGrammarRepository, "source.js", largeJs),
  (),
);

bench(
  ~name="tokenize: Large CSS file (bootstrap)",
  ~options=singleOption,
  ~setup,
  ~f=tokenizeFile(cssGrammarRepository, "source.css", largeCss),
  (),
);
