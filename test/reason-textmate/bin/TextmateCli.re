//
// TextmateCli.re
// 
// Tool for testing the textmate grammars
// Can be run via: `esy @test x TextmateCli --sourceFile=/path/to/source --grammarFile=/path/to/grammar --lineNumber=1 --scope=source.js`


print_endline ("** Welcome to TextmateCli **");

module Cli = {
	type t = {
		sourceFile: string,
		grammarFile: string,
		scope: string,
		lineNumber: int,
	};

	let parse = (args) => {
		sourceFile: "source",
		grammarFile: "grammar",
		scope: "scope",
		lineNumber: 1,
	};


	let toString = ({sourceFile, grammarFile, scope, lineNumber}) => Printf.sprintf({|
Using arguments:
- sourceFile: %s
- grammarFile: %s
- scope: %s
- lineNumber: %d
	|},  sourceFile, grammarFile, scope, lineNumber);
};

let args = Cli.parse(Sys.argv);
print_endline (args |> Cli.toString);
