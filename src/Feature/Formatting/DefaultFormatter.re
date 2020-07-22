open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module Internal = {
  let doFormat = (~indentation, ~languageConfiguration, lines) => {
    let len: int = Array.length(lines);
    let out = Array.copy(lines);

    if (len > 0) {
      let previousLine = ref(lines[0]);
      let beforePreviousLine = ref(None);
      let currentIndentationLevel = ref(None);

      for (idx in 0 to len - 1) {
        let line = lines[idx];

        if (StringEx.isEmpty(line)) {
          out[idx] = "";
        } else {
          let indentLevel =
            switch (currentIndentationLevel^) {
            | None => Indentation.getLevel(indentation, line)
            | Some(indent) =>
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

              indent + increaseIndentAmount + decreaseIndentAmount;
            };

          out[idx] =
            Indentation.applyLevel(
              ~indentation,
              ~level=indentLevel,
              lines[idx],
            );

          currentIndentationLevel := Some(indentLevel);
          beforePreviousLine := Some(previousLine^);
          previousLine := line;
        };
      };
    };
    out;
  };

  let%test_module "format" =
    (module
     {
       let buffer = lines => lines |> Array.of_list;

       let indent2Spaces =
         IndentationSettings.{mode: Spaces, size: 2, tabSize: 2};
       let indent3Spaces =
         IndentationSettings.{mode: Spaces, size: 3, tabSize: 3};

       let indentTabs = IndentationSettings.{mode: Tabs, size: 1, tabSize: 2};

       let languageConfiguration = LanguageConfiguration.default;

       let%test "empty array" = {
         buffer([])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [||];
       };

       let%test "no indent" = {
         buffer(["abc"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"abc"|];
       };

       let%test "simple indent" = {
         buffer(["{", "abc"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"{", "  abc"|];
       };

       let%test "increase / decrease indent" = {
         buffer(["{", "abc", "}"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"{", "  abc", "}"|];
       };

       let%test "2-spaces replaced with 3-spaces" = {
         buffer(["{", "  abc", "}"])
         |> doFormat(~indentation=indent3Spaces, ~languageConfiguration)
         == [|"{", "   abc", "}"|];
       };

       let%test "spaces replaced with tabs" = {
         buffer(["{", "  abc", "}"])
         |> doFormat(~indentation=indentTabs, ~languageConfiguration)
         == [|"{", "\tabc", "}"|];
       };

       let%test "tabs replaced with spaces" = {
         buffer(["{", "\tabc", "}"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"{", "  abc", "}"|];
       };

       let%test "initial whitespace is ignored" = {
         buffer(["      ", "{", "abc", "}"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"", "{", "  abc", "}"|];
       };

       let%test "extraneous whitespace is ignored" = {
         buffer(["{", "      ", "abc", "}"])
         |> doFormat(~indentation=indent2Spaces, ~languageConfiguration)
         == [|"{", "", "  abc", "}"|];
       };

       let%test "increase / decrease indent (tabs)" = {
         buffer(["{", "abc", "}"])
         |> doFormat(~indentation=indentTabs, ~languageConfiguration)
         == [|"{", "\tabc", "}"|];
       };
     });
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
