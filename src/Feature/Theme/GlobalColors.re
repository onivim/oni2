// VSCode theme defaults and fallbacks defined in (amongst other places)
// https://github.com/microsoft/vscode/blob/634068a42471d610453d97fa0c81ec7e713c4e17/src/vs/platform/theme/common/colorRegistry.ts
// https://github.com/microsoft/vscode/blob/abb01b183d450a590cbb78acb4e787eec5445830/src/vs/workbench/common/theme.ts
// https://github.com/microsoft/vscode/blob/d49c5f3bc73ca4b41fe1306b2bf1d5b4bff96291/src/vs/editor/common/view/editorColorRegistry.ts
// https://github.com/microsoft/vscode/blob/0991720b7b44ffc15760b578b284caff78ccf398/src/vs/workbench/contrib/terminal/common/terminalColorRegistry.ts
// https://github.com/microsoft/vscode/blob/418d1974ca0f99529e4f55c8d05f0a202404c980/src/vs/workbench/contrib/scm/browser/dirtydiffDecorator.ts
// https://github.com/microsoft/vscode/blob/3b192336931fa3deaac15233005ba44b51865c19/src/vs/editor/contrib/suggest/suggestWidget.ts

open Oni_Core;
open ColorTheme.Schema;

let foreground =
  define(
    "foreground",
    {light: hex("#CCC"), dark: hex("#616161"), hc: hex("#FFF")},
  );
let focusBorder =
  define(
    "focusBorder",
    {
      dark: transparent(0.8, hex("#0E639C")),
      light: transparent(0.4, hex("#007ACC")),
      hc: hex("#F38518"),
    },
  );
let contrastBorder =
  define(
    "contrastBorder",
    {light: unspecified, dark: unspecified, hc: hex("#6FC3DF")},
  );
let activeContrastBorder =
  define(
    "activeContrastBorder",
    {light: unspecified, dark: unspecified, hc: ref(focusBorder)},
  );

module ActivityBar = {
  let background =
    define(
      "activityBar.background",
      {dark: hex("#333"), light: hex("#2C2C2C"), hc: hex("#000")},
    );
  let foreground = define("activityBar.foreground", hex("#fff") |> all);
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
  let background =
    define(
      "editor.background",
      {light: hex("#fffffe"), dark: hex("#1E1E1E"), hc: hex("#000")},
    );
  let foreground =
    define(
      "editor.foreground",
      {light: hex("#333"), dark: hex("#BBB"), hc: hex("#FFF")},
    );

  let findMatchBackground =
    define(
      "editor.findMatchBackground",
      {light: hex("#A8AC94"), dark: hex("#515C6A"), hc: unspecified},
    );
  let findMatchBorder =
    define(
      "editor.findMatchBorder",
      {light: unspecified, dark: unspecified, hc: ref(activeContrastBorder)},
    );
  let findMatchHighlightsBackground =
    define(
      "editor.findMatchHighlightBackground",
      {light: hex("#EA5C0055"), dark: hex("#EA5C0055"), hc: unspecified},
    );
  let lineHighlightBackground =
    define("editor.lineHighlightBackground", all(unspecified));
  let selectionBackground =
    define(
      "editor.selectionBackground",
      {light: hex("#ADD6FF"), dark: hex("#264F78"), hc: hex("#f3f518")},
    );

  let defaults = [
    background,
    foreground,
    findMatchBackground,
    findMatchBorder,
    findMatchHighlightsBackground,
    lineHighlightBackground,
    selectionBackground,
  ];
};

module EditorCursor = {
  let foreground =
    define(
      "editorCursor.foreground",
      {dark: hex("#AEAFAD"), light: hex("#000"), hc: hex("#FFF")},
    );
  let background =
    define("editorCursor.background", all(opposite(ref(foreground)))); // actually: all(unspecified)

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

module EditorGutter = {
  let background =
    define("editorGutter.background", ref(Editor.background) |> all);
  let modifiedBackground =
    define(
      "editorGutter.modifiedBackground",
      {
        dark: rgb(12, 125, 157),
        light: rgb(102, 175, 224),
        hc: rgb(0, 155, 249),
      },
    );
  let addedBackground =
    define(
      "editorGutter.addedBackground",
      {
        dark: rgb(88, 124, 12),
        light: rgb(129, 184, 139),
        hc: rgb(51, 171, 78),
      },
    );
  let deletedBackground =
    define(
      "editorGutter.deletedBackground",
      {
        dark: rgb(148, 21, 27),
        light: rgb(129, 184, 139),
        hc: rgb(51, 171, 78),
      },
    );

  let defaults = [
    background,
    modifiedBackground,
    addedBackground,
    deletedBackground,
  ];
};

module EditorWidget = {
  let background =
    define(
      "editorWidget.background",
      {dark: hex("#252526"), light: hex("#F3F3F3"), hc: hex("#0C141F")},
    );
  let foreground = define("editorWidget.foreground", all(ref(foreground)));
  let border =
    define(
      "editorWidget.border",
      {
        dark: hex("#454545"),
        light: hex("#C8C8C8"),
        hc: ref(contrastBorder),
      },
    );

  let defaults = [background, foreground, border];
};

module EditorHoverWidget = {
  let background =
    define(
      "editorHoverWidget.background",
      all(ref(EditorWidget.background)),
    );
  let foreground =
    define(
      "editorHoverWidget.foreground",
      all(ref(EditorWidget.foreground)),
    );
  let border =
    define("editorHoverWidget.border", all(ref(EditorWidget.border)));

  let defaults = [background, foreground, border];
};

module EditorWhitespace = {
  let foreground =
    define("editorWhitespace.foreground", hex("#3b4048") |> all);

  let defaults = [foreground];
};

module EditorIndentGuide = {
  let background =
    define(
      "editorIndentGuide.background",
      all(ref(EditorWhitespace.foreground)),
    );
  let activeBackground =
    define(
      "editorIndentGuide.activeBackground",
      all(ref(EditorWhitespace.foreground)),
    );

  let defaults = [background, activeBackground];
};

module EditorLineNumber = {
  module Deprecated = {
    let activeForeground =
      define(
        "editorActiveLineNumber.activeForeground",
        {
          dark: hex("#c6c6c6"),
          light: hex("#08216F"),
          hc: ref(activeContrastBorder),
        },
      );
  };

  let foreground =
    define(
      "editorLineNumber.foreground",
      {dark: hex("#858585"), light: hex("#237893"), hc: hex("#FFF")},
    );
  let activeForeground =
    define(
      "editorLineNumber.activeForeground",
      all(ref(Deprecated.activeForeground)),
    );

  let defaults = [foreground, activeForeground, Deprecated.activeForeground];
};

module EditorOverviewRuler = {
  let bracketMatchForeground =
    define(
      "editorOverviewRuler.bracketMatchForeground",
      all(hex("#7f7f7f4d")),
    );

  let defaults = [bracketMatchForeground];
};

module EditorRuler = {
  let foreground =
    define(
      "editorRuler.foreground",
      {dark: hex("#5A5A5A"), light: rgb(211, 211, 211), hc: hex("#FFF")},
    );

  let defaults = [foreground];
};

module List = {
  let focusBackground =
    define(
      "list.focusBackground",
      {dark: hex("#062F4A"), light: hex("#D6EBFF"), hc: unspecified},
    );
  let focusForeground = define("list.focusForeground", all(unspecified));
  let hoverBackground =
    define(
      "list.hoverBackground",
      {dark: hex("#2A2D2E"), light: hex("#F0F0F0"), hc: unspecified},
    );
  let hoverForeground = define("list.hoverForeground", all(unspecified));
  let highlightForeground =
    define(
      "list.highlightForeground",
      {dark: hex("#0097fb"), light: hex("#0066BF"), hc: ref(focusBorder)},
    );

  let defaults = [
    focusBackground,
    focusForeground,
    hoverBackground,
    hoverForeground,
  ];
};

module EditorSuggestWidget = {
  let background =
    define(
      "editorSuggestWidget.background",
      all(ref(EditorWidget.background)),
    );
  let border =
    define("editorSuggestWidget,border", all(ref(EditorWidget.border)));
  let foreground =
    define("editorSuggestWidget.background", all(ref(Editor.foreground)));
  let highlightForeground =
    define(
      "editorSuggestWidget.highlightForeground",
      all(ref(List.highlightForeground)),
    );
  let selectedBackground =
    define(
      "editorSuggestWidget.selectedBackground",
      all(ref(List.focusBackground)),
    );

  let defaults = [
    background,
    border,
    highlightForeground,
    selectedBackground,
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

  let backgroundFor = (mode: Mode.t) =>
    switch (mode) {
    | Select
    | TerminalVisual
    | Visual => visualModeBackground
    | CommandLine => commandlineModeBackground
    | Operator => operatorModeBackground
    | TerminalInsert
    | Insert => insertModeBackground
    | Replace => replaceModeBackground
    | TerminalNormal
    | Normal => normalModeBackground
    };

  let foregroundFor = (mode: Mode.t) =>
    switch (mode) {
    | Select
    | TerminalVisual
    | Visual => visualModeForeground
    | CommandLine => commandlineModeForeground
    | Operator => operatorModeForeground
    | TerminalInsert
    | Insert => insertModeForeground
    | Replace => replaceModeForeground
    | TerminalNormal
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

module ScrollbarSlider = {
  let background =
    define(
      "scrollbarSlider.background",
      {
        dark: transparent(0.4, hex("#797979")),
        light: transparent(0.4, hex("#646464")),
        hc: transparent(0.6, ref(contrastBorder)),
      },
    );
  let hoverBackground =
    define(
      "scrollbarSlider.hoverBackground",
      {
        dark: transparent(0.7, hex("#646464")),
        light: transparent(0.7, hex("#646464")),
        hc: transparent(0.8, ref(contrastBorder)),
      },
    );
  let activeBackground =
    define(
      "scrollbarSlider.activeBackground",
      {
        dark: transparent(0.3, hex("#BFBFBF")),
        light: transparent(0.6, hex("#000")),
        hc: ref(contrastBorder),
      },
    );

  let defaults = [background, activeBackground, hoverBackground];
};

module SideBar = {
  let background =
    define(
      "sidebar.background",
      {dark: hex("#252526"), light: hex("#F3F3F3"), hc: hex("#000")},
    );
  let foreground = define("sidebar.foreground", all(unspecified));

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
  define("list.activeSelectionBackground", hex("#495162") |> all),
  define("list.activeSelectionForeground", hex("#FFFFFF") |> all),
  define("menu.background", hex("#2F3440") |> all),
  define("menu.foreground", hex("#FFFFFF") |> all),
  define("menu.selectionBackground", hex("#495162") |> all),
  define(
    "statusBar.background",
    {dark: hex("#007aCC"), light: hex("#007aCC"), hc: unspecified},
  ),
  define("statusBar.foreground", hex("#fff") |> all),
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
];
