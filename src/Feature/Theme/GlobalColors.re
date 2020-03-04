open Oni_Core;
open Revery;

module Editor = {
  let background = "editor.background";
  let foreground = "editor.foreground";

  let defaults =
    ColorTheme.Defaults.[
      (background, hex("#2F3440") |> uniform),
      (foreground, hex("#DCDCDC") |> uniform),
    ];
};

module EditorGroupHeader = {
  let tabsBackground = "editorGroupHeader.tabsBackground";

  let defaults =
    ColorTheme.Defaults.[
      (
        tabsBackground,
        {dark: hex("#252526"), light: hex("#F3F3F3"), hc: unspecified},
      ),
    ];
};

module List = {
  let focusBackground = "list.focusBackground";
  let focusForeground = "list.focusForeground";
  let hoverBackground = "list.hoverBackground";
  let hoverForeground = "list.hoverForeground";

  let defaults =
    ColorTheme.Defaults.[
      (focusBackground, hex("#495162") |> uniform),
      (focusForeground, hex("#FFFFFF") |> uniform),
      (hoverBackground, hex("#495162") |> uniform),
      (hoverForeground, hex("#FFFFFF") |> uniform),
    ];
};

module Oni = {
  let visualModeBackground = "oni.visualModeBackground";
  let insertModeBackground = "oni.insertModeBackground";
  let replaceModeBackground = "oni.replaceModeBackground";
  let normalModeBackground = "oni.normalModeBackground";
  let operatorModeBackground = "oni.operatorModeBackground";
  let commandlineModeBackground = "oni.commandlineModeBackground";
  let visualModeForeground = "oni.visualModeForeground";
  let insertModeForeground = "oni.insertModeForeground";
  let replaceModeForeground = "oni.replaceModeForeground";
  let normalModeForeground = "oni.normalModeForeground";
  let operatorModeForeground = "oni.operatorModeForeground";
  let commandlineModeForeground = "oni.commandlineModeForeground";

  let backgroundFor = (mode: Vim.Mode.t) =>
    switch (mode) {
    | Visual => visualModeBackground
    | CommandLine => commandlineModeBackground
    | Operator => operatorModeBackground
    | Insert => insertModeBackground
    | Replace => replaceModeBackground
    | Normal => normalModeBackground
    };

  let foregroundFor = (mode: Vim.Mode.t) =>
    switch (mode) {
    | Visual => visualModeForeground
    | CommandLine => commandlineModeForeground
    | Operator => operatorModeForeground
    | Insert => insertModeForeground
    | Replace => replaceModeForeground
    | Normal => normalModeForeground
    };

  let defaults =
    ColorTheme.Defaults.[
      (visualModeBackground, hex("#56b6c2") |> uniform),
      (insertModeBackground, hex("#98c379") |> uniform),
      (replaceModeBackground, hex("#d19a66") |> uniform),
      (normalModeBackground, hex("#61afef") |> uniform),
      (operatorModeBackground, hex("#d19a66") |> uniform),
      (commandlineModeBackground, hex("#61afef") |> uniform),
      (visualModeForeground, hex("#282c34") |> uniform),
      (insertModeForeground, hex("#282c34") |> uniform),
      (replaceModeForeground, hex("#282c34") |> uniform),
      (normalModeForeground, hex("#282c34") |> uniform),
      (operatorModeForeground, hex("#282c34") |> uniform),
      (commandlineModeForeground, hex("#282c34") |> uniform),
    ];
};

module SideBar = {
  let background = "sidebar.background";
  let foreground = "sidebar.foreground";

  let defaults =
    ColorTheme.Defaults.[
      (background, hex("#21252b") |> uniform),
      (foreground, hex("#ECEFF4") |> uniform),
    ];
};

module Tab = {
  let activeBackground = "tab.activeBackground";
  let activeForeground = "tab.activeForeground";
  let inactiveBackground = "tab.inactiveBackground";
  let inactiveForeground = "tab.inactiveForeground";

  let defaults =
    ColorTheme.Defaults.[
      (
        activeBackground,
        {
          dark: ref(Editor.background),
          light: ref(Editor.background),
          hc: ref(Editor.background),
        },
      ),
      (
        activeForeground,
        {dark: hex("#fff"), light: hex("#333"), hc: hex("#fff")},
      ),
      (
        inactiveBackground,
        {dark: hex("#2D2D2D"), light: hex("#ECECEC"), hc: unspecified},
      ),
      (
        inactiveForeground,
        {
          dark: transparent(0.4, ref(activeForeground)),
          light: transparent(0.4, ref(activeForeground)),
          hc: hex("#fff"),
        },
      ),
    ];
};

let remaining =
  ColorTheme.Defaults.[
    ("foreground", hex("#ECEFF4") |> uniform),
    ("activityBar.background", hex("#2F3440") |> uniform),
    ("activityBar.foreground", hex("#fff") |> uniform),
    ("activityBar.inactiveForeground", hex("#DCDCDC") |> uniform),
    ("activityBar.activeBackground", hex("#fff0") |> uniform),
    ("activityBar.activeBorder", hex("#fff") |> uniform),
    ("editorCursor.background", hex("#2F3440") |> uniform),
    ("editorCursor.foreground", hex("#DCDCDC") |> uniform),
    ("editor.findMatchBackground", hex("#42557b") |> uniform),
    ("editor.findMatchBorder", hex("#457dff") |> uniform),
    ("editor.findMatchHighlightBackground", hex("#314365") |> uniform),
    ("editor.hoverWidgetBackground", hex("#FFFFFF") |> uniform),
    ("editor.hoverWidgetBorder", hex("#FFFFFF") |> uniform),
    ("editor.lineHighlightBackground", hex("#495162") |> uniform),
    //("editorLineNumberBackground", hex("#2F3440")),
    ("editorLineNumber.foreground", hex("#495162") |> uniform),
    ("editorLineNumber.activeForeground", hex("#737984") |> uniform),
    (
      "editorRuler.foreground",
      color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> uniform,
    ),
    ("editorSuggestWidget.background", hex("#282C35") |> uniform),
    ("editorSuggestWidget,border", hex("#ECEFF4") |> uniform),
    ("editorSuggestWidget.highlightForeground", hex("#ECEFF4") |> uniform),
    ("editorSuggestWidget.selectedBackground", hex("#282C35") |> uniform),
    (
      "editorOverviewRuler.bracketMatchForeground",
      hex("#A0A0A0") |> uniform,
    ),
    //("editorctiveLineNumberForeground", hex("#737984")),
    ("editor.selectionBackground", hex("#687595") |> uniform),
    ("list.activeSelectionBackground", hex("#495162") |> uniform),
    ("list.activeSelectionForeground", hex("#FFFFFF") |> uniform),
    (
      "scrollbarSlider.background",
      color(Color.rgba(0., 0., 0., 0.2)) |> uniform,
    ),
    ("scrollbarSlider.activeBackground", hex("#2F3440") |> uniform),
    ("editorIndentGuide.background", hex("#3b4048") |> uniform),
    (
      "editorIndentGuide.activeBackground",
      color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> uniform,
    ),
    ("menu.background", hex("#2F3440") |> uniform),
    ("menu.foreground", hex("#FFFFFF") |> uniform),
    ("menu.selectionBackground", hex("#495162") |> uniform),
    ("editorWhitespace.foreground", hex("#3b4048") |> uniform),
    (
      "statusBar.background",
      {dark: hex("#007aCC"), light: hex("#007aCC"), hc: unspecified},
    ),
    ("statusBar.foreground", hex("#fff") |> uniform),
    (
      "scrollbarSlider.hoverBackground",
      color(Color.rgba(123.0, 123.0, 123.0, 0.1)) |> uniform,
    ),
    //("notificationSuccessBackground", hex("#23d160")),
    //("notificationSuccessForeground", hex("#fff")),
    ("notification.infoBackground", hex("#209cee") |> uniform),
    ("notification.infoForeground", hex("#fff") |> uniform),
    ("notification.warningBackground", hex("#ffdd57") |> uniform),
    ("notification.warningForeground", hex("#fff") |> uniform),
    ("notification.errorBackground", hex("#ff3860") |> uniform),
    ("notification.errorForeground", hex("#fff") |> uniform),
    //("sneakBackground", Revery.Colors.red),
    //("sneakForeground", hex("#fff")),
    //("sneakHighlight", hex("#fff")),
    ("titleBar.activeBackground", hex("#282C35") |> uniform),
    ("titleBar.activeForeground", hex("#ECEFF4") |> uniform),
    ("titleBar.inactiveBackground", hex("#282C35") |> uniform),
    ("titleBar.inactiveForeground", hex("#ECEFF4") |> uniform),
    ("titleBar.border", hex("#fff0") |> uniform),
    ("editorGutter.modifiedBackground", hex("#0C7D9D") |> uniform),
    ("editorGutter.addedBackground", hex("#587C0C") |> uniform),
    ("editorGutter.deletedBackground", hex("#94151B") |> uniform),
  ];
