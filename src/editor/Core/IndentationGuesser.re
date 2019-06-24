/*
 * IndentationGuesser.re
 */

let getLeadingWhitespace = (s: string) => {
  let len = String.length(s);
  let i = ref(0);

  let foundChar = ref(false);
  let spaceCount = ref(0);
  let tabCount = ref(0);
  while (i^ < len && foundChar^ == false) {
    let c = s.[i^];
    if (c == ' ') {
      incr(spaceCount);
    } else if (c == '\t') {
      incr(tabCount);
    } else {
      foundChar := true;
    };
    incr(i);
  };

  (spaceCount^, tabCount^, foundChar^);
};

type t = {
  mode: IndentationSettings.mode,
  size: int,
};

let getMaxKey = (map: IntMap.t(int)) => {
  let (maxKey, _) =
    IntMap.fold(
      (key, v, curr) => {
        let (curMaxKey, curMaxVal) = curr;

		 print_endline ("Get max key - key: " ++ string_of_int(key) ++ " val: " ++ string_of_int(v));
        if (v > curMaxVal || curMaxKey == (-1)) {
          (key, v);
        } else {
          (curMaxKey, curMaxVal);
        };
      },
      map,
      ((-1), (-1)),
    );

  maxKey;
};

let guessIndentation =
    (
      ~f: int => string,
      lineCount: int,
      defaultTabSize: int,
      defaultInsertSpaces: bool,
    ) => {
  let linesToCheck = min(lineCount, 1000);
  let i = ref(0);

  /* Map of the delta between spaces in lines */
  let spaceDelta = ref(IntMap.empty);
  let linesWithLeadingSpaces = ref(0);
  let linesWithLeadingTabs = ref(0);
  while (i^ < linesToCheck) {
    let idx = i^;
    let line = f(idx);

    let prevLine =
      switch (idx) {
      | 0 => ""
      | v => f(v - 1)
      };

    let (spaceCount, tabCount, foundChar) = getLeadingWhitespace(line);

    /* Only consider lines with non-whitespace */
    if (foundChar) {
      if (spaceCount > 0) {
        incr(linesWithLeadingSpaces);
      };

      if (tabCount > 0) {
        incr(linesWithLeadingTabs);
      };

      let (prevSpaceCount, _, prevFoundChar) =
        getLeadingWhitespace(prevLine);
      if (prevFoundChar) {
        let diff = abs(prevSpaceCount - spaceCount);
	    if (diff > 0) {
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
	    }
      };
    };

    incr(i);
  };

  let linesWithSpaces = linesWithLeadingSpaces^;
  let linesWithTabs = linesWithLeadingTabs^;

  let shouldInsertSpaces =
    if (linesWithSpaces == linesWithTabs) {
      defaultInsertSpaces;
    } else if (linesWithSpaces > linesWithTabs) {
      true;
    } else {
      false;
    };

  let size =
    switch (shouldInsertSpaces) {
    | false => defaultTabSize
    | true => let max = getMaxKey(spaceDelta^);
    if (max > 0) {
    	max
    } else {
    	defaultTabSize
    }
    };

  let mode =
    shouldInsertSpaces ? IndentationSettings.Spaces : IndentationSettings.Tabs;
  {mode, size};
};

let guessIndentationArray =
    (lines: array(string), defaultTabSize: int, defaultInsertSpaces: bool) => {
  let f = i => lines[i];
  guessIndentation(
    ~f,
    Array.length(lines),
    defaultTabSize,
    defaultInsertSpaces,
  );
};
