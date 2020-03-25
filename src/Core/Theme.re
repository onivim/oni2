/*
 * Theme.re
 *
 * Theming color / info
 */

// VSCode theme defaults and fallbacks defined in
// https://github.com/microsoft/vscode/blob/634068a42471d610453d97fa0c81ec7e713c4e17/src/vs/platform/theme/common/colorRegistry.ts
// https://github.com/microsoft/vscode/blob/abb01b183d450a590cbb78acb4e787eec5445830/src/vs/workbench/common/theme.ts
// https://github.com/microsoft/vscode/blob/d49c5f3bc73ca4b41fe1306b2bf1d5b4bff96291/src/vs/editor/common/view/editorColorRegistry.ts
// https://github.com/microsoft/vscode/blob/0991720b7b44ffc15760b578b284caff78ccf398/src/vs/workbench/contrib/terminal/common/terminalColorRegistry.ts

open Revery;

type defaults = {
  foreground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorIndentGuideBackground: Color.t,
  editorIndentGuideActiveBackground: Color.t,
  editorWhitespaceForeground: Color.t,
};

let light: defaults = {
  foreground: Color.hex("#616161"),
  editorBackground: Color.hex("#fffffe"),
  editorForeground: Color.hex("#333"),
  editorIndentGuideBackground: Color.hex("#D3D3D3"),
  editorIndentGuideActiveBackground: Color.hex("#939393"),
  editorWhitespaceForeground: Color.hex("#33333333"),
};

let dark: defaults = {
  foreground: Color.hex("#ccc"),
  editorBackground: Color.hex("#1E1E1E"),
  editorForeground: Color.hex("#bbb"),
  editorIndentGuideBackground: Color.hex("#404040"),
  editorIndentGuideActiveBackground: Color.hex("#707070"),
  editorWhitespaceForeground: Color.hex("#e3e4e229"),
};

let hcDark: defaults = {
  foreground: Color.hex("#fff"),
  editorBackground: Color.hex("#000"),
  editorForeground: Color.hex("#FFF"),
  editorIndentGuideBackground: Color.hex("#FFF"),
  editorIndentGuideActiveBackground: Color.hex("#FFF"),
  editorWhitespaceForeground: Color.hex("#e3e4e229"),
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
  foreground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorCursorBackground: Color.t,
  editorCursorForeground: Color.t,
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
  foreground: Color.hex("#ECEFF4"),
  sideBarBackground: Color.hex("#21252b"),
  sideBarForeground: Color.hex("#ECEFF4"),
  editorBackground: Color.hex("#2F3440"),
  editorForeground: Color.hex("#DCDCDC"),
  editorCursorBackground: Color.hex("#2F3440"),
  editorCursorForeground: Color.hex("#DCDCDC"),
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

  let emptyString = "";
  let getColor = (default, items) => {
    let colorString = getFirstOrDefault(~default=emptyString, items, ct);

    if (colorString != emptyString) {
      Color.hex(colorString);
    } else {
      default;
    };
  };

  let foreground = getColor(defaults.foreground, ["foreground"]);

  let editorBackground =
    getColor(defaults.editorBackground, ["editor.background"]);
  let editorForeground =
    getColor(defaults.foreground, ["editor.foreground", "foreground"]);

  let editorCursorBackground =
    getColor(
      defaults.editorBackground,
      ["editorCursor.background", "editor.background"],
    );
  let editorCursorForeground =
    getColor(
      defaults.foreground,
      ["editorCursor.foreground", "editor.foreground", "foreground"],
    );

  let editorHoverWidgetBackground =
    getColor(
      defaults.editorBackground,
      ["editorHoverWidget.background", "editor.background"],
    );

  let editorHoverWidgetBorder =
    getColor(
      defaults.foreground,
      ["editorHoverWidget.border", "editor.foreground", "foreground"],
    );

  let editorLineNumberBackground =
    getColor(
      defaults.editorBackground,
      ["editorLineNumber.background", "editor.background"],
    );
  let editorLineNumberForeground =
    getColor(
      defaults.foreground,
      ["editorLineNumber.foreground", "editor.foreground", "foreground"],
    );

  let editorRulerForeground =
    getColor(
      defaults.foreground,
      ["editorRuler.foreground", "editor.foreground", "foreground"],
    );

  let editorLineHighlightBackground =
    getColor(
      defaults.editorBackground,
      ["editor.lineHighlightBackground", "editor.background"],
    );

  let editorSuggestWidgetBackground =
    getColor(
      defaults.editorBackground,
      ["editorSuggestWidget.background", "editor.background"],
    );

  let editorSuggestWidgetSelectedBackground =
    getColor(
      defaults.editorBackground,
      ["editorSuggestWidget.selectedBackground", "editor.background"],
    );

  let editorSuggestWidgetBorder =
    getColor(
      defaults.editorBackground,
      [
        "editorSuggestWidget.border",
        "editorHoverWidget.border",
        "editorSuggestWidget.background",
        "editor.background",
      ],
    );
  let editorSuggestWidgetHighlightForeground =
    getColor(
      defaults.foreground,
      [
        "editorSuggestWidget.highlightForeground",
        "editor.foreground",
        "foreground",
      ],
    );

  let editorWhitespaceForeground =
    getColor(
      defaults.editorWhitespaceForeground,
      ["editorWhitespace.foreground"],
    );

  let editorIndentGuideBackground =
    getColor(
      defaults.editorIndentGuideBackground,
      ["editorIndentGuide.background"],
    );

  let editorIndentGuideActiveBackground =
    getColor(
      defaults.editorIndentGuideActiveBackground,
      ["editorIndentGuide.activeBackground"],
    );

  let menuBackground =
    getColor(
      defaults === light ? Colors.white : Color.hex("#3c3c3c"),
      ["menu.background", "dropdown.background"],
    );

  let menuForeground =
    getColor(
      defaults === light ? defaults.foreground : Color.hex("#f0f0f0"),
      ["menu.foreground", "dropdown.foreground"],
    );

  let menuSelectionBackground =
    getColor(
      defaults.editorBackground,
      [
        "menu.selectionBackground",
        "list.activeSelectionBackground",
        "editor.selectionHighlightBackground",
        "list.focusBackground",
        "list.hoverBackground",
        "editor.background",
      ],
    );

  let sideBarBackground =
    getColor(defaults.editorBackground, ["sideBar.background"]);

  let sideBarForeground =
    getColor(
      defaults.foreground, // Actually `null`
      ["sideBar.foreground"],
    );

  let scrollbarSliderActiveBackground =
    getColor(
      defaults === light ? Color.hex("#00000094") : Color.hex("#bfbfbf64"),
      ["scrollbarSlider.activeBackground"],
    );

  let scrollbarSliderBackground =
    getColor(
      defaults === light ? Color.hex("#64646464") : Color.hex("#79797964"),
      ["scrollbarSlider.background"],
    );

  let scrollbarSliderHoverBackground =
    getColor(
      defaults === light ? Color.hex("#646464B4") : Color.hex("#646464B4"),
      ["scrollbarSlider.hoverBackground"],
    );

  let sneakBackground = menuSelectionBackground;
  let sneakForeground = menuForeground;
  let sneakHighlight = default.oniNormalModeBackground;

  let titleBarActiveBackground =
    getColor(
      defaults.editorBackground,
      ["titleBar.activeBackground", "editor.background"],
    );

  let titleBarInactiveBackground =
    getColor(
      defaults.editorBackground,
      [
        "titleBar.inactiveBackground",
        "titleBar.activeBackground",
        "editor.background",
      ],
    );

  let titleBarActiveForeground =
    getColor(
      defaults.foreground,
      ["titleBar.activeForeground", "editor.foreground"],
    );

  let titleBarInactiveForeground =
    getColor(
      defaults.foreground,
      [
        "titleBar.inactiveForeground",
        "titlebar.activeForeground",
        "foreground",
        "editor.foreground",
      ],
    );

  let titleBarBorder = getColor(Color.hex("#0000"), ["titleBar.border"]);

  {
    ...default,
    foreground,
    editorBackground,
    editorForeground,
    editorCursorBackground,
    editorCursorForeground,
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
    editorWhitespaceForeground,
    listActiveSelectionBackground:
      getColor(
        defaults === light ? Color.hex("#0074E8") : Color.hex("#094771"),
        ["list.activeSelectionBackground"],
      ),
    listActiveSelectionForeground:
      getColor(Colors.white, ["list.activeSelectionForeground"]),
    listFocusBackground:
      getColor(
        defaults === light ? Color.hex("#D6EBFF") : Color.hex("#062F4A"),
        ["list.focusBackground"],
      ),
    listFocusForeground:
      getColor(
        defaults.foreground, // Actually `null`, but not sure what that means
        ["list.focusForeground"],
      ),
    listHoverBackground:
      getColor(
        defaults === light ? Color.hex("#F0F0F0") : Color.hex("#2A2D2E"),
        ["list.hoverBackground"],
      ),
    listHoverForeground:
      getColor(
        defaults.foreground, // Actually `null`, but not sure what that means
        ["list.hoverForeground"],
      ),
    menuBackground,
    menuForeground,
    menuSelectionBackground,
    scrollbarSliderActiveBackground,
    scrollbarSliderBackground,
    scrollbarSliderHoverBackground,
    sideBarBackground,
    sideBarForeground,
    sneakBackground,
    sneakForeground,
    sneakHighlight,
    tabActiveForeground:
      getColor(
        defaults === light ? Color.hex("#333") : Colors.white,
        ["tab.activeForeground"],
      ),
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
    | Select
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
