/*
 * IndentationGuesser.re
 */
open Oni_Core;

module Constants = {
  // The minimum numbers of spaces for considering it indentation
  let minSpaces = 2;
};

type t = {
  mode: IndentationSettings.mode,
  size: int,
};

let getMaxKey = (map: IntMap.t(int)) => {
  let (maxKey, _) =
    IntMap.fold(
      (key, v, (curMaxKey, curMaxVal)) =>
        if (v > curMaxVal || curMaxKey == (-1)) {
          (key, v);
        } else {
          (curMaxKey, curMaxVal);
        },
      map,
      ((-1), (-1)),
    );

  maxKey;
};

let guessIndentation =
    (
      ~f as getLine: int => string,
      lineCount: int,
      defaultTabSize: int,
      defaultInsertSpaces: bool,
    ) => {
  let linesToCheck = min(lineCount, 1000);

  /* Map of the delta between spaces in lines */
  let spaceDelta = ref(IntMap.empty);
  let linesWithLeadingSpaces = ref(0);
  let linesWithLeadingTabs = ref(0);

  for (i in 0 to linesToCheck - 1) {
    let line = getLine(i);
    let prevLine = i == 0 ? "" : getLine(i - 1);

    let (spaceCount, tabCount, foundChar) =
      Indentation.getLeadingWhitespace(line);

    /* Only consider lines with non-whitespace */
    if (foundChar) {
      if (spaceCount >= Constants.minSpaces) {
        incr(linesWithLeadingSpaces);
      };

      if (tabCount > 0) {
        incr(linesWithLeadingTabs);
      };

      let (prevSpaceCount, _, prevFoundChar) =
        Indentation.getLeadingWhitespace(prevLine);
      if (prevFoundChar) {
        let diff = abs(prevSpaceCount - spaceCount);
        if (diff >= Constants.minSpaces) {
          spaceDelta :=
            IntMap.update(
              diff,
              curr =>
                switch (curr) {
                | Some(v) => Some(v + 1)
                | None => Some(1)
                },
              spaceDelta^,
            );
        };
      };
    };
  };

  if (linesWithLeadingSpaces^ == 0 && linesWithLeadingTabs^ == 0) {
    None;
  } else {
    let shouldInsertSpaces =
      if (linesWithLeadingSpaces^ == linesWithLeadingTabs^) {
        defaultInsertSpaces;
      } else {
        linesWithLeadingSpaces^ > linesWithLeadingTabs^;
      };

    let mode =
      shouldInsertSpaces
        ? IndentationSettings.Spaces : IndentationSettings.Tabs;

    let size =
      if (shouldInsertSpaces) {
        let max = getMaxKey(spaceDelta^);
        if (max >= Constants.minSpaces) {
          max;
        } else {
          defaultTabSize;
        };
      } else {
        defaultTabSize;
      };

    Some({mode, size});
  };
};

let guessIndentationArray =
    (lines: array(string), defaultTabSize: int, defaultInsertSpaces: bool) =>
  guessIndentation(
    ~f=i => lines[i],
    Array.length(lines),
    defaultTabSize,
    defaultInsertSpaces,
  );

let%test_module "guessIndentation" =
  (module
   {
     let indeterminateLines = [|"abc", "def", "ghi"|];

     let moreTabsThanSpaces = [|"\tabc", "  def", "\tghi"|];

     let someEmptyLines = [|"  ", "\tdef", "  "|];

     let moreSpacesThanTabs = [|"  abc", "  def", "  ghi"|];

     let mostlySingleSpaced = [|
       " abc",
       "  def",
       "   ghi",
       "  ghi",
       "    ghi",
       "   ghi",
     |];

     let mostlyDoubleSpaced = [|
       "  abc",
       "    def",
       "  ghi",
       "  ghi",
       "   ghi",
       "      ghi",
     |];

     let mostlyTripleSpaced = [|
       "   abc",
       "      def",
       "   ghi",
       "   ghi",
       "     ghi",
       "        ghi",
     |];

     let largerExample = [|
       "module Index = {",
       "  [@deriving show({with_path: false})]",
       "  type t =",
       "    | ZeroBasedIndex(int)",
       "    | OneBasedIndex(int);",
       "",
       "  let toZeroBasedInt = (pos: t) =>",
       "    switch (pos) {",
       "    | ZeroBasedIndex(n) => n",
       "    | OneBasedIndex(n) => n - 1",
       "    };",
       "",
       "  let toInt0 = toZeroBasedInt;",
       "",
       "  let toOneBasedInt = (pos: t) =>",
       "    switch (pos) {",
       "    | ZeroBasedIndex(n) => n + 1",
       "    | OneBasedIndex(n) => n",
       "    };",
       "",
       "  let toInt1 = toOneBasedInt;",
       "};",
       "",
       "module EditorSize = {",
       "  [@deriving show({with_path: false})]",
       "  type t = {",
       "    pixelWidth: int,",
       "    pixelHeight: int,",
       "  };",
       "",
       "  let create = (~pixelWidth: int, ~pixelHeight: int, ()) => {",
       "    pixelWidth,",
       "    pixelHeight,",
       "  };",
       "};",
     |];

     let singleSpaceBlockComment = [|
       "/*****",
       " *",
       " * Foo",
       " *",
       " *",
       " */",
     |];

     let%test "indeterminate should be None" = {
       guessIndentationArray(indeterminateLines, 4, true) == None
       && guessIndentationArray(indeterminateLines, 3, false) == None;
     };

     let%test "more tabs than spaces" = {
       guessIndentationArray(moreTabsThanSpaces, 4, true)
       == Some(IndentationSettings.{mode: Tabs, size: 4});
     };

     let%test "more spaces than tabs" = {
       guessIndentationArray(moreSpacesThanTabs, 4, false)
       == Some(IndentationSettings.{mode: Spaces, size: 4});
     };

     let%test "ignores empty lines" = {
       guessIndentationArray(someEmptyLines, 4, false)
       == Some(IndentationSettings.{mode: Tabs, size: 4});
     };

     let%test "mostly single spaced" = {
       guessIndentationArray(mostlySingleSpaced, 4, false)
       == Some(IndentationSettings.{mode: Spaces, size: 2});
     };

     let%test "mostly double spaced" = {
       guessIndentationArray(mostlyDoubleSpaced, 4, false)
       == Some(IndentationSettings.{mode: Spaces, size: 2});
     };

     let%test "mostly triple spaced" = {
       guessIndentationArray(mostlyTripleSpaced, 4, false)
       == Some(IndentationSettings.{mode: Spaces, size: 3});
     };

     let%test "larger example" = {
       guessIndentationArray(largerExample, 4, false)
       == Some(IndentationSettings.{mode: Spaces, size: 2});
     };

     let%test "single-space block comment" = {
       guessIndentationArray(singleSpaceBlockComment, 4, false) == None;
     };
   });
