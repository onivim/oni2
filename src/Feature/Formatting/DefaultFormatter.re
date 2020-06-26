open EditorCoreTypes;
open Oni_Core;

let format = (~indentation, ~languageConfiguration, ~startLineNumber, lines) => {
  let rec loop = (currentLineIdx, lines, edits) => {
    switch (lines) {
    | [] => edits
    | [hd, ...tail] => 
		let edit = Vim.Edit.{
			range: Range.{
				start: { line: currentLineIdx, column: Index.zero},
				stop: { line: currentLineIdx, column: Index.zero}
			},
			text: [|"abc"|]
		};
		loop(Index.(currentLineIdx+1), tail, [edit, ...edits]);
    };
  };

  loop(startLineNumber, lines, []);
};
