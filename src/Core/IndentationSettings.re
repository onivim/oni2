/*
 * IndentationSettings.re
 */

[@deriving show]
type mode =
  | Tabs
  | Spaces;

[@deriving show]
type t = {
  mode,
  size: int,
  tabSize: int,
};

let default = {mode: Spaces, size: 4, tabSize: 4};

let create = (~mode, ~size, ~tabSize, ()) => {mode, size, tabSize};

let isEqual = (a: t, b: t) => {
  a.mode == b.mode && a.size == b.size && a.tabSize == b.tabSize;
};

let normalizeTabs = (~indentation: t, str: string) =>
  if (indentation.mode == Tabs) {
    str;
  } else {
    let rec leadingTabCount = (acc, idx) =>
      if (idx >= String.length(str)) {
        acc;
      } else if (str.[idx] == '\t') {
        leadingTabCount(acc + 1, idx + 1);
      } else {
        acc;
      };

    let indentationSpaces = String.make(indentation.size, ' ');

    let tabCount = leadingTabCount(0, 0);
    let indentation =
      List.init(tabCount, _idx => indentationSpaces) |> String.concat("");

    let stringWithoutTabs =
      String.sub(str, tabCount, String.length(str) - tabCount);
    indentation ++ stringWithoutTabs;
  };

let%test_module "normalizeTabs" =
  (module
   {
     let indent2Spaces = {mode: Spaces, size: 2, tabSize: 2};
     let indent2Tabs = {mode: Tabs, size: 2, tabSize: 2};
     let%test "no tabs" = {
       normalizeTabs(~indentation=indent2Spaces, "abc") == "abc";
     };
     let%test "single tab" = {
       normalizeTabs(~indentation=indent2Spaces, "\tabc") == "  abc";
     };
     let%test "multiple tabs" = {
       normalizeTabs(~indentation=indent2Spaces, "\t\tabc") == "    abc";
     };
     let%test "preserves tabs w/ tab indent" = {
       normalizeTabs(~indentation=indent2Tabs, "\t\tabc") == "\t\tabc";
     };
   });
