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

type t = {
  background: Color.t,
  foreground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorHoverWidgetBackground: Color.t,
  editorHoverWidgetBorder: Color.t,
  editorLineHighlightBackground: Color.t,
  editorLineNumberBackground: Color.t,
  editorLineNumberForeground: Color.t,
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
  menuBackground: Color.t,
  menuForeground: Color.t,
  menuSelectionBackground: Color.t,
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
  editorHoverWidgetBackground: Color.hex("#FFFFFF"),
  editorHoverWidgetBorder: Color.hex("#FFFFFF"),
  editorLineHighlightBackground: Color.hex("#495162"),
  editorLineNumberBackground: Color.hex("#2F3440"),
  editorLineNumberForeground: Color.hex("#495162"),
  editorSuggestWidgetBackground: Color.hex("#282C35"),
  editorSuggestWidgetBorder: Color.hex("#ECEFF4"),
  editorSuggestWidgetHighlightForeground: Color.hex("#ECEFF4"),
  editorSuggestWidgetSelectedBackground: Color.hex("#282C35"),
  editorOverviewRulerBracketMatchForeground: Color.hex("#A0A0A0"),
  editorActiveLineNumberForeground: Color.hex("#737984"),
  editorSelectionBackground: Color.hex("#687595"),
  scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
  scrollbarSliderActiveBackground: Color.hex("#2F3440"),
  editorIndentGuideBackground: Color.hex("#3b4048"),
  editorIndentGuideActiveBackground: Color.rgba(0.78, 0.78, 0.78, 0.78),
  menuBackground: Color.hex("#2F3440"),
  menuForeground: Color.hex("#FFFFFF"),
  menuSelectionBackground: Color.hex("#495162"),
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

let ofColorTheme = (uiTheme, ct: Textmate.ColorTheme.t) => {
  open Textmate.ColorTheme;
  let defaults = getDefaults(uiTheme);
  let defaultBackground = defaults.editorBackground;
  let defaultForeground = defaults.editorForeground;

  let getColor = (default, items) => {
    let colorString = getFirstOrDefault(~default, items, ct);
    Color.hex(colorString);
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

  let editorLineHighlightBackground =
    getColor(
      defaultBackground,
      ["editor.lineHighlightBackground", "editor.background", "background"],
    );

  let editorSuggestWidgetBackground =
    getColor(
      defaultBackground,
      ["editorSuggestWidget.background", "editor.background", "background"]
    );
  
  let editorSuggestWidgetSelectedBackground =
    getColor(
      defaultBackground,
      ["editorSuggestWidget.selectedBackground", "editor.background", "background"]
    );
  
  let editorSuggestWidgetBorder =
    getColor(
      defaultBackground,
      ["editorSuggestWidget.border", "editorSuggestWidget.background", "editor.background", "background"]
    );
  let editorSuggestWidgetHighlightForeground =
    getColor(defaultForeground, ["editorSuggestWidget.highlightForeground", "editor.foreground", "foreground"]);

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
        "list.focusBackground",
        "list.hoverBackground",
        "menu.background",
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

  {
    ...default,
    background,
    foreground,
    editorBackground,
    editorForeground,
    editorHoverWidgetBackground,
    editorHoverWidgetBorder,
    editorIndentGuideBackground,
    editorIndentGuideActiveBackground,
    editorLineHighlightBackground,
    editorLineNumberForeground,
    editorLineNumberBackground,
    editorSuggestWidgetBackground,
    editorSuggestWidgetBorder,
    editorSuggestWidgetHighlightForeground,
    editorSuggestWidgetSelectedBackground,
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
