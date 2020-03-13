open Oni_Core;
open Revery;
open ColorTheme.Schema;

let foreground =
  define(
    "foreground",
    {light: hex("#CCC"), dark: hex("#616161"), hc: hex("#FFF")},
  );
let contrastBorder =
  define(
    "contrastBorder",
    {light: unspecified, dark: unspecified, hc: hex("#6FC3DF")},
  );

module ActivityBar = {
  let background =
    define(
      "activityBar.background",
      {dark: hex("#333"), light: hex("#2C2C2C"), hc: hex("#000")},
    );
  let foreground = define("acitvityBar.foreground", hex("#fff") |> uniform);
  let border =
    define(
      "activityBar.border",
      {dark: unspecified, light: unspecified, hc: ref(contrastBorder)},
    );
  let activeBorder =
    define(
      "activityBar.activeBorder",
      {dark: ref(foreground), light: ref(foreground), hc: unspecified},
    );
  let inactiveForeground =
    define(
      "activityBar.inactiveForeground",
      {
        dark: transparent(0.4, ref(foreground)),
        light: transparent(0.4, ref(foreground)),
        hc: hex("#FFF"),
      },
    );
  let activeBackground =
    define("activityBar.activeBackground", unspecified |> uniform);

  let defaults = [
    background,
    foreground,
    border,
    activeBorder,
    inactiveForeground,
    activeBackground,
  ];
};

module Editor = {
  let background = define("editor.background", hex("#2F3440") |> uniform);
  let foreground = define("editor.foreground", hex("#DCDCDC") |> uniform);

  let defaults = [background, foreground];
};

module EditorGroupHeader = {
  let tabsBackground =
    define(
      "editorGroupHeader.tabsBackground",
      {dark: hex("#252526"), light: hex("#F3F3F3"), hc: unspecified},
    );

  let defaults = [tabsBackground];
};

module List = {
  let focusBackground =
    define("list.focusBackground", hex("#495162") |> uniform);
  let focusForeground =
    define("list.focusForeground", hex("#FFFFFF") |> uniform);
  let hoverBackground =
    define("list.hoverBackground", hex("#495162") |> uniform);
  let hoverForeground =
    define("list.hoverForeground", hex("#FFFFFF") |> uniform);

  let defaults = [
    focusBackground,
    focusForeground,
    hoverBackground,
    hoverForeground,
  ];
};

module Oni = {
  let visualModeBackground =
    define("oni.visualModeBackground", hex("#56b6c2") |> uniform);
  let insertModeBackground =
    define("oni.insertModeBackground", hex("#98c379") |> uniform);
  let replaceModeBackground =
    define("oni.replaceModeBackground", hex("#d19a66") |> uniform);
  let normalModeBackground =
    define("oni.normalModeBackground", hex("#61afef") |> uniform);
  let operatorModeBackground =
    define("oni.operatorModeBackground", hex("#d19a66") |> uniform);
  let commandlineModeBackground =
    define("oni.commandlineModeBackground", hex("#61afef") |> uniform);
  let visualModeForeground =
    define("oni.visualModeForeground", hex("#282c34") |> uniform);
  let insertModeForeground =
    define("oni.insertModeForeground", hex("#282c34") |> uniform);
  let replaceModeForeground =
    define("oni.replaceModeForeground", hex("#282c34") |> uniform);
  let normalModeForeground =
    define("oni.normalModeForeground", hex("#282c34") |> uniform);
  let operatorModeForeground =
    define("oni.operatorModeForeground", hex("#282c34") |> uniform);
  let commandlineModeForeground =
    define("oni.commandlineModeForeground", hex("#282c34") |> uniform);

  let backgroundFor = (mode: Vim.Mode.t, theme) =>
    (
      switch (mode) {
      | Visual => visualModeBackground
      | CommandLine => commandlineModeBackground
      | Operator => operatorModeBackground
      | Insert => insertModeBackground
      | Replace => replaceModeBackground
      | Normal => normalModeBackground
      }
    ).
      get(
      theme,
    );

  let foregroundFor = (mode: Vim.Mode.t, theme) =>
    (
      switch (mode) {
      | Visual => visualModeForeground
      | CommandLine => commandlineModeForeground
      | Operator => operatorModeForeground
      | Insert => insertModeForeground
      | Replace => replaceModeForeground
      | Normal => normalModeForeground
      }
    ).
      get(
      theme,
    );

  let defaults = [
    visualModeBackground,
    insertModeBackground,
    replaceModeBackground,
    normalModeBackground,
    operatorModeBackground,
    commandlineModeBackground,
    visualModeForeground,
    insertModeForeground,
    replaceModeForeground,
    normalModeForeground,
    operatorModeForeground,
    commandlineModeForeground,
  ];
};

module SideBar = {
  let background = define("sidebar.background", hex("#21252b") |> uniform);
  let foreground = define("sidebar.foreground", hex("#ECEFF4") |> uniform);

  let defaults = [background, foreground];
};

module Tab = {
  // BACKGROUND

  let activeBackground =
    define(
      "tab.activeBackground",
      {
        dark: ref(Editor.background),
        light: ref(Editor.background),
        hc: ref(Editor.background),
      },
    );
  let unfocusedActiveBackground =
    define(
      "tab.unfocusedActiveBackground",
      ref(activeBackground) |> uniform,
    );
  let inactiveBackground =
    define(
      "tab.inactiveBackground",
      {dark: hex("#2D2D2D"), light: hex("#ECECEC"), hc: unspecified},
    );
  let hoverBackground = define("tab.hoverBackground", unspecified |> uniform);
  let unfocusedHoverBackground =
    define(
      "tab.unfocusedHoverBackground",
      {
        dark: transparent(0.5, ref(hoverBackground)),
        light: transparent(0.7, ref(hoverBackground)),
        hc: unspecified,
      },
    );

  // BORDER

  let border =
    define(
      "tab.border",
      {
        dark: hex("#252526"),
        light: hex("#F3F3F3"),
        hc: ref(contrastBorder),
      },
    );
  let activeBorder = define("tab.activeBorder", unspecified |> uniform);
  let unfocusedActiveBorder =
    define(
      "tab.unfocusedActiveBorder",
      {
        dark: transparent(0.5, ref(activeBorder)),
        light: transparent(0.7, ref(activeBorder)),
        hc: unspecified,
      },
    );
  let activeBorderTop = define("tab.activeBorderTop", unspecified |> uniform);
  let unfocusedActiveBorderTop =
    define(
      "tab.unfocusedActiveBorderTop",
      {
        dark: transparent(0.5, ref(activeBorderTop)),
        light: transparent(0.7, ref(activeBorderTop)),
        hc: unspecified,
      },
    );
  let activeModifiedBorder =
    define(
      "tab.activeModifiedBorder",
      {
        dark: hex("#252526"),
        light: hex("#F3F3F3"),
        hc: ref(contrastBorder),
      },
    );
  let inactiveModifiedBorder =
    define(
      "tab.inactiveModifiedBorder",
      {
        dark: transparent(0.5, ref(activeModifiedBorder)),
        light: transparent(0.7, ref(activeModifiedBorder)),
        hc: hex("#FFF"),
      },
    );

  let unfocusedActiveModifiedBorder =
    define(
      "tab.unfocusedActiveModifiedBorder",
      {
        dark: transparent(0.5, ref(activeModifiedBorder)),
        light: transparent(0.7, ref(activeModifiedBorder)),
        hc: hex("#FFF"),
      },
    );
  let unfocusedInactiveModifiedBorder =
    define(
      "tab.unfocusedInactiveModifiedBorder",
      {
        dark: transparent(0.5, ref(inactiveModifiedBorder)),
        light: transparent(0.5, ref(inactiveModifiedBorder)),
        hc: hex("#FFF"),
      },
    );
  let hoverBorder = define("tab.hoverBorder", unspecified |> uniform);
  let unfocusedHoverBorder =
    define(
      "tab.unfocusedHoverBorder",
      {
        dark: transparent(0.5, ref(hoverBorder)),
        light: transparent(0.7, ref(hoverBorder)),
        hc: unspecified,
      },
    );

  // FOREGROUND

  let activeForeground =
    define(
      "tab.activeForeground",
      {dark: hex("#FFF"), light: hex("#333"), hc: hex("#FFF")},
    );
  let inactiveForeground =
    define(
      "tab.inactiveForeground",
      {
        dark: transparent(0.5, ref(activeForeground)),
        light: transparent(0.7, ref(activeForeground)),
        hc: hex("#FFF"),
      },
    );
  let unfocusedActiveForeground =
    define(
      "tab.unfocusedActiveForeground",
      {
        dark: transparent(0.5, ref(activeForeground)),
        light: transparent(0.7, ref(activeForeground)),
        hc: hex("#FFF"),
      },
    );
  let unfocusedInactiveForeground =
    define(
      "tab.unfocusedInactiveForeground",
      {
        dark: transparent(0.5, ref(inactiveForeground)),
        light: transparent(0.5, ref(inactiveForeground)),
        hc: hex("#FFF"),
      },
    );

  let defaults = [
    // BACKGROUND
    activeBackground,
    unfocusedActiveBackground,
    inactiveBackground,
    hoverBackground,
    unfocusedHoverBackground,
    // BORDER
    border,
    activeBorder,
    unfocusedActiveBorder,
    activeBorderTop,
    unfocusedActiveBorderTop,
    activeModifiedBorder,
    inactiveModifiedBorder,
    unfocusedActiveModifiedBorder,
    unfocusedInactiveModifiedBorder,
    hoverBorder,
    unfocusedHoverBorder,
    // FOREGROUND
    activeForeground,
    inactiveForeground,
    unfocusedActiveForeground,
    unfocusedInactiveForeground,
  ];
};

let defaults = [foreground, contrastBorder];

let remaining = [
  define("editorCursor.background", hex("#2F3440") |> uniform),
  define("editorCursor.foreground", hex("#DCDCDC") |> uniform),
  define("editor.findMatchBackground", hex("#42557b") |> uniform),
  define("editor.findMatchBorder", hex("#457dff") |> uniform),
  define("editor.findMatchHighlightBackground", hex("#314365") |> uniform),
  define("editor.hoverWidgetBackground", hex("#FFFFFF") |> uniform),
  define("editor.hoverWidgetBorder", hex("#FFFFFF") |> uniform),
  define("editor.lineHighlightBackground", hex("#495162") |> uniform),
  //define("editorLineNumberBackground", hex("#2F3440")),
  define("editorLineNumber.foreground", hex("#495162") |> uniform),
  define("editorLineNumber.activeForeground", hex("#737984") |> uniform),
  define(
    "editorRuler.foreground",
    color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> uniform,
  ),
  define("editorSuggestWidget.background", hex("#282C35") |> uniform),
  define("editorSuggestWidget,border", hex("#ECEFF4") |> uniform),
  define(
    "editorSuggestWidget.highlightForeground",
    hex("#ECEFF4") |> uniform,
  ),
  define(
    "editorSuggestWidget.selectedBackground",
    hex("#282C35") |> uniform,
  ),
  define(
    "editorOverviewRuler.bracketMatchForeground",
    hex("#A0A0A0") |> uniform,
  ),
  //define("editorctiveLineNumberForeground", hex("#737984")),
  define("editor.selectionBackground", hex("#687595") |> uniform),
  define("list.activeSelectionBackground", hex("#495162") |> uniform),
  define("list.activeSelectionForeground", hex("#FFFFFF") |> uniform),
  define(
    "scrollbarSlider.background",
    color(Color.rgba(0., 0., 0., 0.2)) |> uniform,
  ),
  define("scrollbarSlider.activeBackground", hex("#2F3440") |> uniform),
  define("editorIndentGuide.background", hex("#3b4048") |> uniform),
  define(
    "editorIndentGuide.activeBackground",
    color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> uniform,
  ),
  define("menu.background", hex("#2F3440") |> uniform),
  define("menu.foreground", hex("#FFFFFF") |> uniform),
  define("menu.selectionBackground", hex("#495162") |> uniform),
  define("editorWhitespace.foreground", hex("#3b4048") |> uniform),
  define(
    "statusBar.background",
    {dark: hex("#007aCC"), light: hex("#007aCC"), hc: unspecified},
  ),
  define("statusBar.foreground", hex("#fff") |> uniform),
  define(
    "scrollbarSlider.hoverBackground",
    color(Color.rgba(123.0, 123.0, 123.0, 0.1)) |> uniform,
  ),
  //define("notificationSuccessBackground", hex("#23d160")),
  //define("notificationSuccessForeground", hex("#fff")),
  define("notification.infoBackground", hex("#209cee") |> uniform),
  define("notification.infoForeground", hex("#fff") |> uniform),
  define("notification.warningBackground", hex("#ffdd57") |> uniform),
  define("notification.warningForeground", hex("#fff") |> uniform),
  define("notification.errorBackground", hex("#ff3860") |> uniform),
  define("notification.errorForeground", hex("#fff") |> uniform),
  //define("sneakBackground", Revery.Colors.red),
  //define("sneakForeground", hex("#fff")),
  //define("sneakHighlight", hex("#fff")),
  define("titleBar.activeBackground", hex("#282C35") |> uniform),
  define("titleBar.activeForeground", hex("#ECEFF4") |> uniform),
  define("titleBar.inactiveBackground", hex("#282C35") |> uniform),
  define("titleBar.inactiveForeground", hex("#ECEFF4") |> uniform),
  define("titleBar.border", hex("#fff0") |> uniform),
  define("editorGutter.modifiedBackground", hex("#0C7D9D") |> uniform),
  define("editorGutter.addedBackground", hex("#587C0C") |> uniform),
  define("editorGutter.deletedBackground", hex("#94151B") |> uniform),
];
