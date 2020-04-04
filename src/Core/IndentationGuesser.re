/*
 * IndentationGuesser.re
 */
open Kernel;

module Constants = {
  // The minimum numbers of spaces for considering it indentation
  let minSpaces = 2;
};

let getLeadingWhitespace = (s: string) => {
  let rec loop = (i, spaces, tabs) =>
    if (i >= String.length(s)) {
      (spaces, tabs, false);
    } else {
      switch (s.[i]) {
      | ' ' => loop(i + 1, spaces + 1, tabs)
      | '\t' => loop(i + 1, spaces, tabs + 1)
      | _ => (spaces, tabs, true)
      };
    };

  loop(0, 0, 0);
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

    let (spaceCount, tabCount, foundChar) = getLeadingWhitespace(line);

    /* Only consider lines with non-whitespace */
    if (foundChar) {
      if (spaceCount >= Constants.minSpaces) {
        incr(linesWithLeadingSpaces);
      };

      if (tabCount > 0) {
        incr(linesWithLeadingTabs);
      };

      let (prevSpaceCount, _, prevFoundChar) =
        getLeadingWhitespace(prevLine);
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

  let shouldInsertSpaces =
    if (linesWithLeadingSpaces^ == linesWithLeadingTabs^) {
      defaultInsertSpaces;
    } else {
      linesWithLeadingSpaces^ > linesWithLeadingTabs^;
    };

  let mode =
    shouldInsertSpaces ? IndentationSettings.Spaces : IndentationSettings.Tabs;

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

  {mode, size};
};

let guessIndentationArray =
    (lines: array(string), defaultTabSize: int, defaultInsertSpaces: bool) =>
  guessIndentation(
    ~f=i => lines[i],
    Array.length(lines),
    defaultTabSize,
    defaultInsertSpaces,
  );
