//
// TextmateCli.re
//
// Tool for testing the textmate grammars
//
// Can be run via:
// `esy @test x TextmateCli --file=package.json --grammar=test/reason-textmate/json.json --line=25

open Textmate;

module Cli = {
  type t = {
    sourceFile: string,
    grammarFile: string,
    lineNumber: int,
  };

  let parse = args => {
    let sourceFile = ref(None);
    let grammarFile = ref(None);
    let lineNumber = ref(0);
    let () =
      Arg.parse_argv(
        ~current=ref(0),
        args,
        [
          (
            "--file",
            String(s => sourceFile := Some(s)),
            "Source file to tokenize",
          ),
          (
            "--grammar",
            String(s => grammarFile := Some(s)),
            "Grammar file to use",
          ),
          ("--line", Int(i => lineNumber := i), "Line number to show"),
        ],
        _ => (),
        "",
      );

    let getRequiredParameter = (name, maybeParam) => {
      switch (maybeParam^) {
      | None =>
        prerr_endline("ERROR: Missing required parameter: " ++ name);
        failwith("Missing parameter: " ++ name);
      | Some(v) => v
      };
    };

    {
      sourceFile: getRequiredParameter("file", sourceFile),
      grammarFile: getRequiredParameter("grammar", grammarFile),
      lineNumber: lineNumber^,
    };
  };

  let toString = ({sourceFile, grammarFile, lineNumber}) =>
    Printf.sprintf(
      {|
Using arguments:
- sourceFile: %s
- grammarFile: %s
- lineNumber: %d
	|},
      sourceFile,
      grammarFile,
      lineNumber,
    );
};

let getOkOrFail = (~msg, res) => {
  switch (res) {
  | Ok(v) => v
  | Error(errMsg) =>
    let err = Printf.sprintf("%s: %s", msg, errMsg);
    prerr_endline(err);
    failwith(err);
  };
};

let run = () => {
  print_endline("** Welcome to TextmateCli **");

  let Cli.{sourceFile, grammarFile, lineNumber} as args =
    Cli.parse(Sys.argv);
  print_endline(args |> Cli.toString);

  print_endline("Loading grammar: " ++ grammarFile);

  let grammar =
    grammarFile
    |> Grammar.Json.of_file
    |> getOkOrFail(~msg="Unable to load grammar " ++ grammarFile);

  let grammarRepository = _ => None;

  let scopeName = Grammar.getScopeName(grammar);
  print_endline("Grammar loaded - using scope: " ++ scopeName);

  print_endline("Trying to read file: " ++ sourceFile);
  let lines =
    sourceFile |> Oni_Core.Utility.File.readAllLines |> Array.of_list;
  print_endline(Printf.sprintf("Read %d lines.", Array.length(lines)));

  let currentScopeStack = ref(None);
  let latestTokens = ref([]);
  let latestLine = ref("");

  for (i in 0 to lineNumber) {
    let line = lines[i];
    let (tokens, scopeStack) =
      Grammar.tokenize(
        ~scopes=currentScopeStack^,
        ~grammarRepository,
        ~grammar,
        lines[i],
      );

    currentScopeStack := Some(scopeStack);
    latestLine := line;
    latestTokens := tokens;
  };

  print_endline(Printf.sprintf("Line %d: %s", lineNumber, latestLine^));
  print_endline("** Tokens: **");

  latestTokens^ |> List.map(Token.show) |> List.iter(print_endline);

  print_endline("Done!");
};

run();
