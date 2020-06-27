open EditorCoreTypes;
open Oni_Core;

module Internal = {
  let doFormat = (~indentation, ~languageConfiguration, lines) => {
    let len: int = Array.length(lines);
    let out = Array.copy(lines);

    if (len > 0) {
      let previousLine = ref(lines[0]);
      let beforePreviousLine = ref(None);
      let currentIndentationlevel =
        ref(Indentation.getLevel(indentation, lines[0]));

      for (idx in 1 to len - 1) {
        let line = lines[idx];

        let increaseIndentAmount =
          LanguageConfiguration.shouldIncreaseIndent(
            ~previousLine=previousLine^,
            ~beforePreviousLine=beforePreviousLine^,
            languageConfiguration,
          )
            ? 1 : 0;

        let decreaseIndentAmount =
          LanguageConfiguration.shouldDecreaseIndent(
            ~line,
            languageConfiguration,
          )
            ? (-1) : 0;

        let newIndentLevel =
          // TODO: Fix this
          currentIndentationlevel^
          + increaseIndentAmount
          + decreaseIndentAmount
          + 1;

        out[idx] =
          Indentation.applyLevel(
            ~indentation,
            ~level=newIndentLevel,
            lines[idx],
          );

        currentIndentationlevel := newIndentLevel;
        beforePreviousLine := Some(previousLine^);
        previousLine := line;
      };
    };
    out;
  };
};

let format = (~indentation, ~languageConfiguration, ~startLineNumber, lines) => {
  let len: int = Array.length(lines);
  let lenIdx =
    Index.toZeroBased(startLineNumber) + len - 1 |> Index.fromZeroBased;
  let out = Internal.doFormat(~indentation, ~languageConfiguration, lines);

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
