/*
 * Theme.re
 *
 * Theming color / info
 */

open Revery;

type t = {
  background: Color.t,
  foreground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorLineHighlightBackground: Color.t,
  editorLineNumberBackground: Color.t,
  editorLineNumberForeground: Color.t,
  editorSelectionBackground: Color.t,
  editorActiveLineNumberForeground: Color.t,
  scrollbarSliderBackground: Color.t,
  scrollbarSliderActiveBackground: Color.t,
  editorFindMatchBackground: Color.t,
  editorFindMatchBorder: Color.t,
  editorFindMatchHighlightBackground: Color.t,
  editorIndentGuideBackground: Color.t,
  editorIndentGuideActiveBackground: Color.t,
  editorMenuBackground: Color.t,
  editorMenuForeground: Color.t,
  editorMenuItemSelected: Color.t,
  editorOverviewRulerBracketMatchForeground: Color.t,
  editorRulerForeground: Color.t,
  editorWhitespaceForeground: Color.t,
  tabActiveForeground: Color.t,
  oniVisualModeBackground: Color.t,
  oniInsertModeBackground: Color.t,
  oniReplaceModeBackground: Color.t,
  oniNormalModeBackground: Color.t,
  oniOperatorModeBackground: Color.t,
  oniCommandlineModeBackground: Color.t,
  oniVisualModeForeground: Color.t,
  oniInsertModeForeground: Color.t,
  oniReplaceModeForeground: Color.t,
  oniNormalModeForeground: Color.t,
  oniOperatorModeForeground: Color.t,
  oniCommandlineModeForeground: Color.t,
  statusBarForeground: Color.t,
  statusBarBackground: Color.t,
  scrollbarSliderHoverBackground: Color.t,
  sideBarBackground: Color.t,
  sideBarForeground: Color.t,
  notificationSuccessBackground: Color.t,
  notificationSuccessForeground: Color.t,
  notificationInfoBackground: Color.t,
  notificationInfoForeground: Color.t,
  notificationWarningBackground: Color.t,
  notificationWarningForeground: Color.t,
  notificationErrorForeground: Color.t,
  notificationErrorBackground: Color.t,
};

let default: t = {
  background: Color.hex("#282C35"),
  foreground: Color.hex("#ECEFF4"),
  sideBarBackground: Color.hex("#21252b"),
  sideBarForeground: Color.hex("#ECEFF4"),
  editorBackground: Color.hex("#2F3440"),
  editorForeground: Color.hex("#DCDCDC"),
  editorFindMatchBackground: Color.hex("#42557b"),
  editorFindMatchBorder: Color.hex("#457dff"),
  editorFindMatchHighlightBackground: Color.hex("#314365"),
  editorLineHighlightBackground: Color.hex("#495162"),
  editorLineNumberBackground: Color.hex("#2F3440"),
  editorLineNumberForeground: Color.hex("#495162"),
  editorOverviewRulerBracketMatchForeground: Color.hex("#A0A0A0"),
  editorActiveLineNumberForeground: Color.hex("#737984"),
  editorSelectionBackground: Color.hex("#687595"),
  scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
  scrollbarSliderActiveBackground: Color.hex("#2F3440"),
  editorIndentGuideBackground: Color.hex("#3b4048"),
  editorIndentGuideActiveBackground: Color.rgba(0.78, 0.78, 0.78, 0.78),
  editorMenuBackground: Color.hex("#2F3440"),
  editorMenuForeground: Color.hex("#FFFFFF"),
  editorMenuItemSelected: Color.hex("#495162"),
  editorRulerForeground: Color.rgba(0.78, 0.78, 0.78, 0.78),
  editorWhitespaceForeground: Color.hex("#3b4048"),
  tabActiveForeground: Color.hex("#DCDCDC"),
  oniVisualModeBackground: Color.hex("#56b6c2"),
  oniInsertModeBackground: Color.hex("#98c379"),
  oniReplaceModeBackground: Color.hex("#d19a66"),
  oniNormalModeBackground: Color.hex("#61afef"),
  oniOperatorModeBackground: Color.hex("#d19a66"),
  oniCommandlineModeBackground: Color.hex("#61afef"),
  oniVisualModeForeground: Color.hex("#282c34"),
  oniInsertModeForeground: Color.hex("#282c34"),
  oniReplaceModeForeground: Color.hex("#282c34"),
  oniNormalModeForeground: Color.hex("#282c34"),
  oniOperatorModeForeground: Color.hex("#282c34"),
  oniCommandlineModeForeground: Color.hex("#282c34"),
  statusBarBackground: Color.hex("#21252b"),
  statusBarForeground: Color.hex("#9da5b4"),
  scrollbarSliderHoverBackground: Color.rgba(123.0, 123.0, 123.0, 0.1),
  notificationSuccessBackground: Color.hex("#23d160"),
  notificationSuccessForeground: Colors.white,
  notificationInfoBackground: Color.hex("#209cee"),
  notificationInfoForeground: Colors.white,
  notificationWarningBackground: Color.hex("#ffdd57"),
  notificationWarningForeground: Colors.white,
  notificationErrorBackground: Color.hex("#ff3860"),
  notificationErrorForeground: Colors.white,
};

let getColorsForMode = (theme: t, mode: Vim.Mode.t) => {
  let (background, foreground) =
    switch (mode) {
    | Visual => (theme.oniVisualModeBackground, theme.oniVisualModeForeground)
    | CommandLine => (
        theme.oniCommandlineModeBackground,
        theme.oniCommandlineModeForeground,
      )
    | Operator => (
        theme.oniOperatorModeBackground,
        theme.oniOperatorModeForeground,
      )
    | Insert => (theme.oniInsertModeBackground, theme.oniInsertModeForeground)
    | Replace => (
        theme.oniReplaceModeBackground,
        theme.oniReplaceModeForeground,
      )
    | Normal => (theme.oniNormalModeBackground, theme.oniNormalModeForeground)
    };

  (background, foreground);
};

type parser = (string, (t, Yojson.Safe.t) => t);

let parseColor: (Color.t, Yojson.Safe.t) => Color.t = (defaultColor, json) => {
  switch (json) {
  | `String(c) => 
    switch(Color.hex(c)) {
    | exception _ => defaultColor
    | v => v
    }
  | _ => defaultColor
  }
};

let parsers: list(parser) = [
  ("background", (s, v) => {...s, background: parseColor(default.background, v) }),
  ("foreground", (s, v) => {...s, foreground: parseColor(default.foreground, v) }),
  ("sideBar.background", (s, v) => {...s, sideBarBackground: parseColor(default.sideBarBackground, v) }),
  ("sideBar.foreground", (s, v) => {...s, sideBarForeground: parseColor(default.sideBarForeground, v) }),
  ("editor.background", (s, v) => {...s, editorBackground: parseColor(default.editorBackground, v) }),
  ("editor.foreground", (s, v) => {...s, editorForeground: parseColor(default.editorForeground, v) }),
  ("editor.lineHighlightBackground", (s, v) => {...s, editorLineHighlightBackground: parseColor(default.editorLineHighlightBackground, v) }),
  ("editorLineNumber.background", (s, v) => {...s, editorLineNumberBackground: parseColor(default.editorLineNumberBackground, v) }),
  ("editorLineNumber.foreground", (s, v) => {...s, editorLineNumberForeground: parseColor(default.editorLineNumberForeground, v) }),
  ("editorLineNumber.activeForeground", (s, v) => {...s, editorActiveLineNumberForeground: parseColor(default.editorActiveLineNumberForeground, v) }),
  ("editor.selectionBackground", (s, v) => {...s, editorSelectionBackground: parseColor(default.editorSelectionBackground, v) }),
  ("statusBar.background", (s, v) => {...s, statusBarBackground: parseColor(default.statusBarBackground, v) }),
  ("statusBar.foreground", (s, v) => {...s, statusBarForeground: parseColor(default.statusBarForeground, v) }),
  
];

let of_yojson = (json: Yojson.Safe.t) => {
  List.fold_left((prev, curr) => {
    let (field, parser) = curr;

    let jsonField = Yojson.Safe.Util.member(field, json)
    parser(prev, jsonField);
  }, default, parsers);
};

