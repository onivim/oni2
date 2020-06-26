open EditorCoreTypes;
open Oni_Core;

let format = (~indentation, ~languageConfiguration, ~startLineNumber, lines) => {
  let len: int = Array.length(lines);
  let lenIdx =
    Index.toZeroBased(startLineNumber) + len - 1 |> Index.fromZeroBased;
  let out = Array.copy(lines);

  if (len == 0) {
    [];
  } else {
    let previousLine = ref(lines[0]);
    let beforePreviousLine = ref(None);
    let _currentIndentationlevel =
      ref(Indentation.getLevel(indentation, lines[0]));

    for (idx in 1 to len - 1) {
      let line = lines[idx];
      // TODO: Actually process indentation...
      out[idx] = "abc " ++ lines[idx];

      beforePreviousLine := Some(previousLine^);
      previousLine := line;
    };

    let edit =
      Vim.Edit.{
        range:
          Range.{
            start: {
              line: startLineNumber,
              column: Index.zero,
            },
            stop: {
              line: lenIdx,
              column: Index.fromZeroBased(Zed_utf8.length(lines[len - 1])),
            },
          },
        text: out,
      };

    [edit];
  };
};
