//
// TextmateCli.re
//
// Tool for testing the textmate grammars
// Can be run via: `esy @test x TextmateCli --sourceFile=/path/to/source --grammarFile=/path/to/grammar --lineNumber=1 --scope=source.js`

print_endline("** Welcome to TextmateCli **");

module Cli = {
  type t = {
    sourceFile: string,
    grammarFile: string,
    scope: string,
    lineNumber: int,
  };

  let parse = args => {
    let sourceFile = ref(None);
    let grammarFile = ref(None);
    let scope = ref(None);
    let lineNumber = ref(0);
    let () = Arg.parse_argv(
      ~current=ref(0),
      args,
      [
         ("--file", String(s => sourceFile := Some(s)), "Source file to tokenize"),
         ("--grammar", String(s => grammarFile := Some(s)), "Grammar file to use"),
         ("--scope", String(s => scope := Some(s)), "Top-level scope (ie, 'source.js')"),
         ("--line", Int(i => lineNumber := i), "Line number to show")
      ],
      _ => (),
      ""
    );

    let getRequiredParameter = (name, maybeParam) => {
      switch(maybeParam^) {
      | None =>
        prerr_endline ("ERROR: Missing required parameter: " ++ name)
        failwith("Missing parameter: " ++ name);
      | Some(v) => v
      }
    };

    {
    sourceFile: getRequiredParameter("file", sourceFile),
    grammarFile: getRequiredParameter("grammar", grammarFile),
    scope: getRequiredParameter("scope", scope),
    lineNumber: lineNumber^
    };
  };

  let toString = ({sourceFile, grammarFile, scope, lineNumber}) =>
    Printf.sprintf(
      {|
Using arguments:
- sourceFile: %s
- grammarFile: %s
- scope: %s
- lineNumber: %d
	|},
      sourceFile,
      grammarFile,
      scope,
      lineNumber,
    );
};

let args = Cli.parse(Sys.argv);
print_endline(args |> Cli.toString);
