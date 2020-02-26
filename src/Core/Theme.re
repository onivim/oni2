/*
 * Theme.re
 *
 * Theming color / info
 */

open Revery;

type defaults = {
  editorBackground: string,
  editorForeground: string,
  editorIndentGuideBackground: string,
  editorIndentGuideActiveBackground: string,
};

let light: defaults = {
  editorBackground: "#FFF",
  editorForeground: "#000",
  editorIndentGuideBackground: "#D3D3D3",
  editorIndentGuideActiveBackground: "#939393",
};

let dark: defaults = {
  editorBackground: "#1E1E1E",
  editorForeground: "#D4D4D4",
  editorIndentGuideBackground: "#404040",
  editorIndentGuideActiveBackground: "#707070",
};
let hcDark: defaults = {
  editorBackground: "#000",
  editorForeground: "#FFF",
  editorIndentGuideBackground: "#FFF",
  editorIndentGuideActiveBackground: "#FFF",
};

let getDefaults = uiTheme =>
  switch (uiTheme) {
  | "vs" => light
  | "vs-light" => light
  | "vs-dark" => dark
  | "hc-black" => hcDark
  | _ => dark
  };

type editorGutter = {
  background: Color.t,
  modifiedBackground: Color.t,
  addedBackground: Color.t,
  deletedBackground: Color.t,
  commentRangeForeground: Color.t,
};

type t = {
  background: Color.t,
  foreground: Color.t,
  activityBarBackground: Color.t,
  activityBarForeground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorHoverWidgetBackground: Color.t,
  editorHoverWidgetBorder: Color.t,
  editorLineHighlightBackground: Color.t,
  editorLineNumberBackground: Color.t,
  editorLineNumberForeground: Color.t,
  editorRulerForeground: Color.t,
  editorSelectionBackground: Color.t,
  editorSuggestWidgetBackground: Color.t,
  editorSuggestWidgetBorder: Color.t,
  editorSuggestWidgetHighlightForeground: Color.t,
  editorSuggestWidgetSelectedBackground: Color.t,
  editorActiveLineNumberForeground: Color.t,
  scrollbarSliderBackground: Color.t,
  scrollbarSliderActiveBackground: Color.t,
  editorFindMatchBackground: Color.t,
  editorFindMatchBorder: Color.t,
  editorFindMatchHighlightBackground: Color.t,
  editorIndentGuideBackground: Color.t,
  editorIndentGuideActiveBackground: Color.t,
  listActiveSelectionBackground: Color.t,
  listActiveSelectionForeground: Color.t,
  listFocusBackground: Color.t,
  listFocusForeground: Color.t,
  listHoverBackground: Color.t,
  listHoverForeground: Color.t,
  menuBackground: Color.t,
  menuForeground: Color.t,
  menuSelectionBackground: Color.t,
  editorOverviewRulerBracketMatchForeground: Color.t,
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
  sneakBackground: Color.t,
  sneakForeground: Color.t,
  sneakHighlight: Color.t,
  terminalBackground: Color.t,
  terminalForeground: Color.t,
  terminalAnsiBlack: Color.t,
  terminalAnsiRed: Color.t,
  terminalAnsiGreen: Color.t,
  terminalAnsiYellow: Color.t,
  terminalAnsiBlue: Color.t,
  terminalAnsiMagenta: Color.t,
  terminalAnsiCyan: Color.t,
  terminalAnsiWhite: Color.t,
  terminalAnsiBrightBlack: Color.t,
  terminalAnsiBrightRed: Color.t,
  terminalAnsiBrightGreen: Color.t,
  terminalAnsiBrightYellow: Color.t,
  terminalAnsiBrightBlue: Color.t,
  terminalAnsiBrightCyan: Color.t,
  terminalAnsiBrightMagenta: Color.t,
  terminalAnsiBrightWhite: Color.t,
  titleBarActiveBackground: Color.t,
  titleBarActiveForeground: Color.t,
  titleBarInactiveBackground: Color.t,
  titleBarInactiveForeground: Color.t,
  titleBarBorder: Color.t,
  editorGutterModifiedBackground: Color.t,
  editorGutterAddedBackground: Color.t,
  editorGutterDeletedBackground: Color.t,
};

let default: t = {
  background: Color.hex("#282C35"),
  foreground: Color.hex("#ECEFF4"),
  sideBarBackground: Color.hex("#21252b"),
  sideBarForeground: Color.hex("#ECEFF4"),
  activityBarBackground: Color.hex("#2F3440"),
  activityBarForeground: Color.hex("#DCDCDC"),
  editorBackground: Color.hex("#2F3440"),
  editorForeground: Color.hex("#DCDCDC"),
  editorFindMatchBackground: Color.hex("#42557b"),
  editorFindMatchBorder: Color.hex("#457dff"),
  editorFindMatchHighlightBackground: Color.hex("#314365"),
  editorHoverWidgetBackground: Color.hex("#FFFFFF"),
  editorHoverWidgetBorder: Color.hex("#FFFFFF"),
  editorLineHighlightBackground: Color.hex("#495162"),
  editorLineNumberBackground: Color.hex("#2F3440"),
  editorLineNumberForeground: Color.hex("#495162"),
  editorRulerForeground: Color.rgba(0.78, 0.78, 0.78, 0.78),
  editorSuggestWidgetBackground: Color.hex("#282C35"),
  editorSuggestWidgetBorder: Color.hex("#ECEFF4"),
  editorSuggestWidgetHighlightForeground: Color.hex("#ECEFF4"),
  editorSuggestWidgetSelectedBackground: Color.hex("#282C35"),
  editorOverviewRulerBracketMatchForeground: Color.hex("#A0A0A0"),
  editorActiveLineNumberForeground: Color.hex("#737984"),
  editorSelectionBackground: Color.hex("#687595"),
  listActiveSelectionBackground: Color.hex("#495162"),
  listActiveSelectionForeground: Color.hex("#FFFFFF"),
  listFocusBackground: Color.hex("#495162"),
  listFocusForeground: Color.hex("#FFFFFF"),
  listHoverBackground: Color.hex("#495162"),
  listHoverForeground: Color.hex("#FFFFFF"),
  scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
  scrollbarSliderActiveBackground: Color.hex("#2F3440"),
  editorIndentGuideBackground: Color.hex("#3b4048"),
  editorIndentGuideActiveBackground: Color.rgba(0.78, 0.78, 0.78, 0.78),
  menuBackground: Color.hex("#2F3440"),
  menuForeground: Color.hex("#FFFFFF"),
  menuSelectionBackground: Color.hex("#495162"),
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
  sneakBackground: Colors.red,
  sneakForeground: Colors.white,
  sneakHighlight: Colors.white,
  terminalBackground: Color.rgb_int(0, 0, 0),
  terminalForeground: Color.rgb_int(233, 235, 235),
  terminalAnsiBlack: Color.rgb_int(0, 0, 0),
  terminalAnsiRed: Color.rgb_int(194, 54, 33),
  terminalAnsiGreen: Color.rgb_int(37, 188, 36),
  terminalAnsiYellow: Color.rgb_int(173, 173, 39),
  terminalAnsiBlue: Color.rgb_int(73, 46, 225),
  terminalAnsiMagenta: Color.rgb_int(211, 56, 211),
  terminalAnsiCyan: Color.rgb_int(51, 197, 200),
  terminalAnsiWhite: Color.rgb_int(203, 204, 205),
  terminalAnsiBrightBlack: Color.rgb_int(129, 131, 131),
  terminalAnsiBrightRed: Color.rgb_int(252, 57, 31),
  terminalAnsiBrightGreen: Color.rgb_int(49, 231, 34),
  terminalAnsiBrightYellow: Color.rgb_int(234, 236, 35),
  terminalAnsiBrightBlue: Color.rgb_int(88, 51, 255),
  terminalAnsiBrightCyan: Color.rgb_int(20, 240, 240),
  terminalAnsiBrightMagenta: Color.rgb_int(20, 240, 240),
  terminalAnsiBrightWhite: Color.rgb_int(233, 235, 235),
  titleBarActiveBackground: Color.hex("#282C35"),
  titleBarActiveForeground: Color.hex("#ECEFF4"),
  titleBarInactiveBackground: Color.hex("#282C35"),
  titleBarInactiveForeground: Color.hex("#ECEFF4"),
  titleBarBorder: Colors.transparentWhite,
  editorGutterModifiedBackground: Color.hex("#0C7D9D"),
  editorGutterAddedBackground: Color.hex("#587C0C"),
  editorGutterDeletedBackground: Color.hex("#94151B"),
};

let ofColorTheme = (uiTheme, ct: Textmate.ColorTheme.t) => {
  open Textmate.ColorTheme;
  let defaults = getDefaults(uiTheme);
  let defaultBackground = defaults.editorBackground |> Color.hex;
  let defaultForeground = defaults.editorForeground |> Color.hex;

  let emptyString = "";
  let getColor = (default, items) => {
    let colorString = getFirstOrDefault(~default=emptyString, items, ct);

    if (colorString != emptyString) {
      Color.hex(colorString);
    } else {
      default;
    };
  };

  let background =
    getColor(defaultBackground, ["background", "editor.background"]);
  let foreground =
    getColor(defaultForeground, ["foreground", "editor.foreground"]);

  let editorBackground =
    getColor(defaultBackground, ["editor.background", "background"]);
  let editorForeground =
    getColor(defaultForeground, ["editor.foreground", "foreground"]);

  let editorHoverWidgetBackground =
    getColor(
      defaultBackground,
      ["editorHoverWidget.background", "editor.background", "background"],
    );

  let activityBarBackground =
    getColor(
      defaultBackground,
      ["activityBar.background", "editor.background", "background"],
    );

  let activityBarForeground =
    getColor(
      defaultForeground,
      ["activityBar.foreground", "editor.foreground", "foreground"],
    );

  let editorHoverWidgetBorder =
    getColor(
      defaultForeground,
      ["editorHoverWidget.border", "editor.foreground", "foreground"],
    );

  let editorLineNumberBackground =
    getColor(
      defaultBackground,
      ["editorLineNumber.background", "editor.background", "background"],
    );
  let editorLineNumberForeground =
    getColor(
      defaultForeground,
      ["editorLineNumber.foreground", "editor.foreground", "foreground"],
    );

  let editorRulerForeground =
    getColor(
      defaultForeground,
      ["editorRuler.foreground", "editor.foreground", "foreground"],
    );

  let editorLineHighlightBackground =
    getColor(
      defaultBackground,
      ["editor.lineHighlightBackground", "editor.background", "background"],
    );

  let editorSuggestWidgetBackground =
    getColor(
      defaultBackground,
      ["editorSuggestWidget.background", "editor.background", "background"],
    );

  let editorSuggestWidgetSelectedBackground =
    getColor(
      defaultBackground,
      [
        "editorSuggestWidget.selectedBackground",
        "editor.background",
        "background",
      ],
    );

  let editorSuggestWidgetBorder =
    getColor(
      defaultBackground,
      [
        "editorSuggestWidget.border",
        "editorHoverWidget.border",
        "editorSuggestWidget.background",
        "editor.background",
        "background",
      ],
    );
  let editorSuggestWidgetHighlightForeground =
    getColor(
      defaultForeground,
      [
        "editorSuggestWidget.highlightForeground",
        "editor.foreground",
        "foreground",
      ],
    );

  let editorIndentGuideBackground =
    getColor(
      defaults.editorIndentGuideBackground |> Color.hex,
      ["editorIndentGuide.background"],
    );

  let editorIndentGuideActiveBackground =
    getColor(
      defaults.editorIndentGuideActiveBackground |> Color.hex,
      ["editorIndentGuide.activeBackground"],
    );

  let menuBackground =
    getColor(
      defaultBackground,
      ["menu.background", "background", "editor.background"],
    );

  let menuForeground =
    getColor(
      defaultForeground,
      ["menu.foreground", "foreground", "editor.foreground"],
    );

  let menuSelectionBackground =
    getColor(
      defaultBackground,
      [
        "menu.selectionBackground",
        "list.activeSelectionBackground",
        "editor.selectionHighlightBackground",
        "list.focusBackground",
        "list.hoverBackground",
        "background",
        "editor.background",
      ],
    );

  let statusBarBackground =
    getColor(
      defaultBackground,
      ["statusBar.background", "editor.background", "background"],
    );

  let statusBarForeground =
    getColor(
      defaultForeground,
      ["statusBar.foreground", "editor.foreground", "foreground"],
    );

  let sideBarBackground =
    getColor(
      defaultBackground,
      ["sideBar.background", "editor.background", "background"],
    );

  let sideBarForeground =
    getColor(
      defaultForeground,
      ["sideBar.foreground", "editor.foreground", "foreground"],
    );

  let scrollbarSliderActiveBackground =
    getColor(
      defaultBackground,
      [
        "scrollbarSlider.activeBackground",
        "menu.selectionBackground",
        "list.activeSelectionBackground",
      ],
    );

  let scrollbarSliderBackground =
    getColor(
      defaultBackground,
      ["scrollbarSlider.background", "menu.background", "list.background"],
    );

  let scrollbarSliderHoverBackground =
    getColor(
      defaultBackground,
      ["scrollbarSlider.hoverBackground", "scrollbarSlider.background"],
    );

  let sneakBackground = menuSelectionBackground;
  let sneakForeground = menuForeground;
  let sneakHighlight = default.oniNormalModeBackground;

  let titleBarActiveBackground =
    getColor(
      defaultBackground,
      ["titleBar.activeBackground", "background", "editor.background"],
    );

  let titleBarInactiveBackground =
    getColor(
      defaultBackground,
      [
        "titleBar.inactiveBackground",
        "titleBar.activeBackground",
        "background",
        "editor.background",
      ],
    );

  let titleBarActiveForeground =
    getColor(
      defaultForeground,
      ["titleBar.activeForeground", "background", "editor.foreground"],
    );

  let titleBarInactiveForeground =
    getColor(
      defaultForeground,
      [
        "titleBar.inactiveForeground",
        "titlebar.activeForeground",
        "foreground",
        "editor.foreground",
      ],
    );

  let titleBarBorder = getColor(Color.hex("#0000"), ["titleBar.border"]);

  let terminalBackground =
    getColor(
      default.terminalBackground,
      ["terminal.background", "terminal.ansiBlack"],
    );

  let terminalForeground =
    getColor(
      default.terminalForeground,
      ["terminal.foreground", "terminal.ansiBrightWhite"],
    );

  let terminalAnsiBlack =
    getColor(default.terminalAnsiBlack, ["terminal.ansiBlack"]);

  let terminalAnsiRed =
    getColor(default.terminalAnsiRed, ["terminal.ansiRed"]);

  let terminalAnsiGreen =
    getColor(default.terminalAnsiGreen, ["terminal.ansiGreen"]);

  let terminalAnsiYellow =
    getColor(default.terminalAnsiYellow, ["terminal.ansiYellow"]);

  let terminalAnsiBlue =
    getColor(default.terminalAnsiBlue, ["terminal.ansiBlue"]);

  let terminalAnsiMagenta =
    getColor(default.terminalAnsiMagenta, ["terminal.ansiMagenta"]);

  let terminalAnsiCyan =
    getColor(default.terminalAnsiCyan, ["terminal.ansiCyan"]);

  let terminalAnsiWhite =
    getColor(default.terminalAnsiWhite, ["terminal.ansiWhite"]);

  let terminalAnsiBrightBlack =
    getColor(default.terminalAnsiBrightBlack, ["terminal.ansiBrightBlack"]);

  let terminalAnsiBrightRed =
    getColor(default.terminalAnsiBrightRed, ["terminal.ansiBrightRed"]);

  let terminalAnsiBrightGreen =
    getColor(default.terminalAnsiBrightGreen, ["terminal.ansiBrightGreen"]);

  let terminalAnsiBrightYellow =
    getColor(
      default.terminalAnsiBrightYellow,
      ["terminal.ansiBrightYellow"],
    );

  let terminalAnsiBrightBlue =
    getColor(default.terminalAnsiBrightBlue, ["terminal.ansiBrightBlue"]);

  let terminalAnsiBrightMagenta =
    getColor(
      default.terminalAnsiBrightMagenta,
      ["terminal.ansiBrightMagenta"],
    );

  let terminalAnsiBrightCyan =
    getColor(default.terminalAnsiBrightCyan, ["terminal.ansiBrightCyan"]);

  let terminalAnsiBrightWhite =
    getColor(default.terminalAnsiBrightWhite, ["terminal.ansiBrightWhite"]);

  {
    ...default,
    background,
    foreground,
    activityBarBackground,
    activityBarForeground,
    editorBackground,
    editorForeground,
    editorHoverWidgetBackground,
    editorHoverWidgetBorder,
    editorIndentGuideBackground,
    editorIndentGuideActiveBackground,
    editorLineHighlightBackground,
    editorLineNumberForeground,
    editorLineNumberBackground,
    editorRulerForeground,
    editorSuggestWidgetBackground,
    editorSuggestWidgetBorder,
    editorSuggestWidgetHighlightForeground,
    editorSuggestWidgetSelectedBackground,
    listActiveSelectionBackground:
      getColor(
        defaultBackground,
        ["list.activeSelectionBackground", "menu.selectionBackground"],
      ),
    listActiveSelectionForeground:
      getColor(
        defaultForeground,
        ["list.activeSelectionForeground", "menu.foreground"],
      ),
    listFocusBackground:
      getColor(
        defaultBackground,
        ["list.focusBackground", "menu.selectionBackground"],
      ),
    listFocusForeground:
      getColor(
        defaultForeground,
        ["list.focusForeground", "menu.foreground"],
      ),
    listHoverBackground:
      getColor(
        defaultBackground,
        ["list.hoverBackground", "menu.selectionBackground"],
      ),
    listHoverForeground:
      getColor(
        defaultForeground,
        ["list.hoverForeground", "menu.foreground"],
      ),
    menuBackground,
    menuForeground,
    menuSelectionBackground,
    scrollbarSliderActiveBackground,
    scrollbarSliderBackground,
    scrollbarSliderHoverBackground,
    sideBarBackground,
    sideBarForeground,
    statusBarBackground,
    statusBarForeground,
    sneakBackground,
    sneakForeground,
    sneakHighlight,
    terminalBackground,
    terminalForeground,
    terminalAnsiBlack,
    terminalAnsiRed,
    terminalAnsiGreen,
    terminalAnsiYellow,
    terminalAnsiBlue,
    terminalAnsiMagenta,
    terminalAnsiCyan,
    terminalAnsiWhite,
    terminalAnsiBrightBlack,
    terminalAnsiBrightRed,
    terminalAnsiBrightGreen,
    terminalAnsiBrightYellow,
    terminalAnsiBrightBlue,
    terminalAnsiBrightMagenta,
    terminalAnsiBrightCyan,
    terminalAnsiBrightWhite,
    titleBarActiveForeground,
    titleBarActiveBackground,
    titleBarInactiveForeground,
    titleBarInactiveBackground,
    titleBarBorder,
  };
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

let getCustomColor = (color, _theme) =>
  switch (color) {
  | "gitDecoration.addedResourceForeground" => Some(Color.hex("#81b88b"))
  | "gitDecoration.modifiedResourceForeground" => Some(Color.hex("#E2C08D"))
  | "gitDecoration.deletedResourceForeground" => Some(Color.hex("#c74e39"))
  | "gitDecoration.untrackedResourceForeground" => Some(Color.hex("#73C991"))
  | "gitDecoration.ignoredResourceForeground" => Some(Color.hex("#8C8C8C"))
  | "gitDecoration.conflictingResourceForeground" =>
    Some(Color.hex("#6c6cc4"))
  | "gitDecoration.submoduleResourceForeground" => Some(Color.hex("#8db9e2"))
  | _ => None
  };
