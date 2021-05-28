/*
 * Indentation.re
 *
 * Helpers for dealing with indentation level
 */

open Utility;

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

let getLevel = (settings: IndentationSettings.t, text: string) => {
  let tabSize = settings.tabSize;

  let spaceCount = ref(0);
  let indentLevel = ref(0);

  let allWhitespace = ref(true);
  let i = ref(0);
  let len = String.length(text);

  while (i^ < len && allWhitespace^) {
    let c = text.[i^];

    switch (c) {
    | '\t' =>
      incr(indentLevel);
      spaceCount := 0;
    | ' ' =>
      if (spaceCount^ == 0) {
        incr(indentLevel);
      };
      incr(spaceCount);
      if (spaceCount^ == tabSize) {
        spaceCount := 0;
      };
    | _ => allWhitespace := false
    };

    incr(i);
  };

  allWhitespace^ ? 0 : indentLevel^;
};

let applyLevel =
    (~indentation: IndentationSettings.t, ~level: int, str: string) => {
  // Negative levels should not be allowed
  let level = level < 0 ? 0 : level;

  str
  |> StringEx.findNonWhitespace
  |> Option.map(idx => {
       let desiredWhitespace =
         switch (indentation.mode) {
         | Tabs => String.make(level, '\t')
         | Spaces => String.make(level * indentation.size, ' ')
         };

       desiredWhitespace ++ String.sub(str, idx, String.length(str) - idx);
     })
  |> Option.value(~default=str);
};

let%test_module "applyLevel" =
  (module
   {
     let indentation = IndentationSettings.default; // 4 spaces

     let%test "remove whitespace, level 0" = {
       applyLevel(~indentation, ~level=0, "  ab") == "ab";
     };

     let%test "add whitespace, level 0" = {
       applyLevel(~indentation, ~level=1, "ab") == "    ab";
     };

     let%test "remove whitespace, negative level" = {
       applyLevel(~indentation, ~level=-1, "  ab") == "ab";
     };
   });
