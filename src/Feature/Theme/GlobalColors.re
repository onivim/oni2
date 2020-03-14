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
  let foreground = define("acitvityBar.foreground", hex("#fff") |> all);
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
    define("activityBar.activeBackground", all(unspecified));

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
  let background = define("editor.background", hex("#2F3440") |> all);
  let foreground = define("editor.foreground", hex("#DCDCDC") |> all);

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
    define("list.focusBackground", hex("#495162") |> all);
  let focusForeground =
    define("list.focusForeground", hex("#FFFFFF") |> all);
  let hoverBackground =
    define("list.hoverBackground", hex("#495162") |> all);
  let hoverForeground =
    define("list.hoverForeground", hex("#FFFFFF") |> all);

  let defaults = [
    focusBackground,
    focusForeground,
    hoverBackground,
    hoverForeground,
  ];
};

module Oni = {
  let visualModeBackground =
    define("oni.visualModeBackground", hex("#56b6c2") |> all);
  let insertModeBackground =
    define("oni.insertModeBackground", hex("#98c379") |> all);
  let replaceModeBackground =
    define("oni.replaceModeBackground", hex("#d19a66") |> all);
  let normalModeBackground =
    define("oni.normalModeBackground", hex("#61afef") |> all);
  let operatorModeBackground =
    define("oni.operatorModeBackground", hex("#d19a66") |> all);
  let commandlineModeBackground =
    define("oni.commandlineModeBackground", hex("#61afef") |> all);
  let visualModeForeground =
    define("oni.visualModeForeground", hex("#282c34") |> all);
  let insertModeForeground =
    define("oni.insertModeForeground", hex("#282c34") |> all);
  let replaceModeForeground =
    define("oni.replaceModeForeground", hex("#282c34") |> all);
  let normalModeForeground =
    define("oni.normalModeForeground", hex("#282c34") |> all);
  let operatorModeForeground =
    define("oni.operatorModeForeground", hex("#282c34") |> all);
  let commandlineModeForeground =
    define("oni.commandlineModeForeground", hex("#282c34") |> all);

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
  let background = define("sidebar.background", hex("#21252b") |> all);
  let foreground = define("sidebar.foreground", hex("#ECEFF4") |> all);

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
    define("tab.unfocusedActiveBackground", ref(activeBackground) |> all);
  let inactiveBackground =
    define(
      "tab.inactiveBackground",
      {dark: hex("#2D2D2D"), light: hex("#ECECEC"), hc: unspecified},
    );
  let hoverBackground = define("tab.hoverBackground", all(unspecified));
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
  let activeBorder = define("tab.activeBorder", all(unspecified));
  let unfocusedActiveBorder =
    define(
      "tab.unfocusedActiveBorder",
      {
        dark: transparent(0.5, ref(activeBorder)),
        light: transparent(0.7, ref(activeBorder)),
        hc: unspecified,
      },
    );
  let activeBorderTop = define("tab.activeBorderTop", all(unspecified));
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
  let hoverBorder = define("tab.hoverBorder", all(unspecified));
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
  define("editorCursor.background", hex("#2F3440") |> all),
  define("editorCursor.foreground", hex("#DCDCDC") |> all),
  define("editor.findMatchBackground", hex("#42557b") |> all),
  define("editor.findMatchBorder", hex("#457dff") |> all),
  define("editor.findMatchHighlightBackground", hex("#314365") |> all),
  define("editor.hoverWidgetBackground", hex("#FFFFFF") |> all),
  define("editor.hoverWidgetBorder", hex("#FFFFFF") |> all),
  define("editor.lineHighlightBackground", hex("#495162") |> all),
  //define("editorLineNumberBackground", hex("#2F3440")),
  define("editorLineNumber.foreground", hex("#495162") |> all),
  define("editorLineNumber.activeForeground", hex("#737984") |> all),
  define(
    "editorRuler.foreground",
    color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> all,
  ),
  define("editorSuggestWidget.background", hex("#282C35") |> all),
  define("editorSuggestWidget,border", hex("#ECEFF4") |> all),
  define("editorSuggestWidget.highlightForeground", hex("#ECEFF4") |> all),
  define("editorSuggestWidget.selectedBackground", hex("#282C35") |> all),
  define(
    "editorOverviewRuler.bracketMatchForeground",
    hex("#A0A0A0") |> all,
  ),
  //define("editorctiveLineNumberForeground", hex("#737984")),
  define("editor.selectionBackground", hex("#687595") |> all),
  define("list.activeSelectionBackground", hex("#495162") |> all),
  define("list.activeSelectionForeground", hex("#FFFFFF") |> all),
  define(
    "scrollbarSlider.background",
    color(Color.rgba(0., 0., 0., 0.2)) |> all,
  ),
  define("scrollbarSlider.activeBackground", hex("#2F3440") |> all),
  define("editorIndentGuide.background", hex("#3b4048") |> all),
  define(
    "editorIndentGuide.activeBackground",
    color(Color.rgba(0.78, 0.78, 0.78, 0.78)) |> all,
  ),
  define("menu.background", hex("#2F3440") |> all),
  define("menu.foreground", hex("#FFFFFF") |> all),
  define("menu.selectionBackground", hex("#495162") |> all),
  define("editorWhitespace.foreground", hex("#3b4048") |> all),
  define(
    "statusBar.background",
    {dark: hex("#007aCC"), light: hex("#007aCC"), hc: unspecified},
  ),
  define("statusBar.foreground", hex("#fff") |> all),
  define(
    "scrollbarSlider.hoverBackground",
    color(Color.rgba(123.0, 123.0, 123.0, 0.1)) |> all,
  ),
  //define("notificationSuccessBackground", hex("#23d160")),
  //define("notificationSuccessForeground", hex("#fff")),
  define("notification.infoBackground", hex("#209cee") |> all),
  define("notification.infoForeground", hex("#fff") |> all),
  define("notification.warningBackground", hex("#ffdd57") |> all),
  define("notification.warningForeground", hex("#fff") |> all),
  define("notification.errorBackground", hex("#ff3860") |> all),
  define("notification.errorForeground", hex("#fff") |> all),
  //define("sneakBackground", Revery.Colors.red),
  //define("sneakForeground", hex("#fff")),
  //define("sneakHighlight", hex("#fff")),
  define("titleBar.activeBackground", hex("#282C35") |> all),
  define("titleBar.activeForeground", hex("#ECEFF4") |> all),
  define("titleBar.inactiveBackground", hex("#282C35") |> all),
  define("titleBar.inactiveForeground", hex("#ECEFF4") |> all),
  define("titleBar.border", hex("#fff0") |> all),
  define("editorGutter.modifiedBackground", hex("#0C7D9D") |> all),
  define("editorGutter.addedBackground", hex("#587C0C") |> all),
  define("editorGutter.deletedBackground", hex("#94151B") |> all),
];
