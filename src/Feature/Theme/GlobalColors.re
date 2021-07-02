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
    {dark: hex("#CCC"), light: hex("#616161"), hc: hex("#FFF")},
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

let shadow =
  define(
    "shadow",
    {
      light: transparent(0.16, hex("#000F")),
      dark: transparent(0.4, hex("#000F")),
      hc: unspecified,
    },
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

module ActivityBarBadge = {
  let background =
    define(
      "activityBarBadge.background",
      {dark: hex("#333333"), light: hex("#2C2C2C"), hc: hex("#000000")},
    );
  let foreground = define("activityBarBadge.foreground", all(hex("#FFF")));

  let defaults = [background, foreground];
};

module Button = {
  let foreground =
    define(
      "button.foreground",
      {dark: hex("#FFF"), light: hex("#FFF"), hc: hex("#FFF")},
    );

  let background =
    define(
      "button.background",
      {dark: hex("#0E639C"), light: hex("#007ACC"), hc: unspecified},
    );

  let hoverBackground =
    define(
      "button.hoverBackground",
      {dark: ref(background), light: ref(background), hc: unspecified},
    );

  let secondaryForeground =
    define(
      "button.secondaryForeground",
      {dark: hex("#FFF"), light: hex("#FFF"), hc: hex("#FFF")},
    );

  let secondaryBackground =
    define(
      "button.secondaryBackground",
      {dark: hex("#0E639C"), light: hex("#007ACC"), hc: unspecified},
    );

  let secondaryHoverBackground =
    define(
      "button.secondaryHoverBackground",
      {
        dark: ref(secondaryBackground),
        light: ref(secondaryBackground),
        hc: unspecified,
      },
    );

  let defaults = [
    background,
    foreground,
    hoverBackground,
    secondaryForeground,
    secondaryBackground,
    secondaryHoverBackground,
  ];
};

module Dropdown = {
  let background =
    define(
      "dropdown.background",
      {dark: hex("#3C3C3C"), light: hex("#FFF"), hc: hex("#000")},
    );
  let foreground =
    define(
      "dropdown.foreground",
      {dark: hex("#F0F0F0"), light: unspecified, hc: hex("#FFF")},
    );

  let defaults = [background, foreground];
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

  let lightBulbForeground =
    define(
      "editorLightBulb.foreground",
      {light: hex("#DDB100"), dark: hex("#FFCC00"), hc: hex("#FFCC00")},
    );

  let lightBulbAutoFixForeground =
    define(
      "editorLightBulbAutoFix.foreground",
      {light: hex("#007ACC"), dark: hex("#75BEFF"), hc: hex("#75BEFF")},
    );

  let lineHighlightBackground =
    define("editor.lineHighlightBackground", all(unspecified));
  let selectionBackground =
    define(
      "editor.selectionBackground",
      {light: hex("#ADD6FF"), dark: hex("#264F78"), hc: hex("#f3f518")},
    );

  let wordHighlightBackground =
    define(
      "editor.wordHighlightBackground",
      {light: hex("#57575740"), dark: hex("#575757B8"), hc: unspecified},
    );
  let defaults = [
    background,
    foreground,
    findMatchBackground,
    findMatchBorder,
    findMatchHighlightsBackground,
    lightBulbForeground,
    lightBulbAutoFixForeground,
    lineHighlightBackground,
    selectionBackground,
    wordHighlightBackground,
  ];
};

module EditorError = {
  let foreground =
    define(
      "editorError.foreground",
      {dark: hex("#F48771"), light: hex("#E51400"), hc: unspecified},
    );
  let border =
    define(
      "editorError.border",
      {dark: unspecified, light: unspecified, hc: hex("#E47777")},
    );

  let defaults = [foreground, border];
};

module EditorUnnecessaryCode = {
  let border =
    define(
      "editorUnnecessaryCode.border",
      {
        dark: unspecified,
        light: unspecified,
        hc: hex("#fff") |> transparent(0.8),
      },
    );

  let opacity =
    define(
      "editorUnnecessaryCode.opacity",
      {dark: hex("#0007"), light: hex("#000a"), hc: unspecified},
    );

  let defaults = [border, opacity];
};

module EditorWarning = {
  let foreground =
    define(
      "editorWarning.foreground",
      {dark: hex("#CCA700"), light: hex("#E9A700"), hc: unspecified},
    );
  let border =
    define(
      "editorWarning.border",
      {dark: unspecified, light: unspecified, hc: hex("#FFCC00")},
    );

  let defaults = [foreground, border];
};

module EditorInfo = {
  let foreground =
    define(
      "editorInfo.foreground",
      {dark: hex("#75BEFF"), light: hex("#75BEFF"), hc: unspecified},
    );
  let border =
    define(
      "editorInfo.border",
      {dark: unspecified, light: unspecified, hc: hex("#75BEFF")},
    );

  let defaults = [foreground, border];
};

module EditorHint = {
  let foreground =
    define(
      "editorHint.foreground",
      {
        dark: hex("#EEE") |> transparent(0.7),
        light: hex("#6c6c6c"),
        hc: unspecified,
      },
    );
  let border =
    define(
      "editorHint.border",
      {
        dark: unspecified,
        light: unspecified,
        hc: hex("#EEE") |> transparent(0.8),
      },
    );

  let defaults = [foreground, border];
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

module EditorGroup = {
  let border =
    define(
      "editorGroup.border",
      {
        dark: hex("#444444"),
        light: hex("#E7E7E7"),
        hc: ref(contrastBorder),
      },
    );

  let defaults = [border];
};

module EditorGroupHeader = {
  let tabsBackground =
    define(
      "editorGroupHeader.tabsBackground",
      {dark: hex("#252526"), light: hex("#F3F3F3"), hc: unspecified},
    );

  let border = define("editorGroupHeader.border", all(ref(tabsBackground)));

  let defaults = [tabsBackground, border];
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

module Notifications = {
  let background =
    define("notifications.background", all(ref(EditorWidget.background)));
  let foreground =
    define("notifications.foreground", all(ref(EditorWidget.foreground)));

  let headerForeground =
    define("notificationsCenterHeader.foreground", all(ref(foreground)));

  let headerBackground =
    define("notificationsCenterHeader.background", all(ref(background)));

  let headerBorder =
    define("notificationsCenterHeader.border", all(ref(headerBackground)));

  let border = define("notifications.border", all(ref(headerBackground)));

  let errorIcon =
    define(
      "notificationsErrorIcon.foreground",
      all(ref(EditorError.foreground)),
    );
  let warningIcon =
    define(
      "notificationsWarningIcon.foreground",
      all(ref(EditorWarning.foreground)),
    );
  let infoIcon =
    define(
      "notificationsWarningIcon.foreground",
      all(ref(EditorInfo.foreground)),
    );

  let defaults = [
    background,
    foreground,
    headerForeground,
    headerBackground,
    border,
    errorIcon,
    warningIcon,
    infoIcon,
  ];
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

  let inactiveFocusBackground =
    define(
      "list.inactiveFocusBackground",
      all(ref(focusBackground) |> transparent(0.75)),
    );

  let focusForeground =
    define("list.focusForeground", all(ref(foreground))); // actually: unspecified

  let focusOutline = define("list.focusOutline", all(ref(focusBorder)));

  let inactiveFocusOutline =
    define("list.inactiveFocusOutline", all(ref(inactiveFocusBackground)));

  let activeSelectionBackground =
    define(
      "list.activeSelectionBackground",
      {dark: hex("#094771"), light: hex("#0074E8"), hc: unspecified},
    );
  let activeSelectionForeground =
    define(
      "list.activeSelectionForeground",
      {dark: hex("#FFF"), light: hex("#FFF"), hc: unspecified},
    );

  let inactiveSelectionBackground =
    define(
      "list.inactiveSelectionBackground",
      all(ref(activeSelectionBackground) |> transparent(0.25)),
    );
  let inactiveSelectionForeground =
    define(
      "list.inactiveSelectionForeground",
      all(ref(activeSelectionForeground)),
    );

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

  let deemphasizedForeground =
    define("list.deemphasizedForeground", all(ref(foreground)));

  let activeIndentGuide =
    define(
      "tree.indentGuidesStroke",
      all(ref(foreground) |> transparent(0.5)),
    );

  let inactiveIndentGuide =
    define(
      "tree.inactive.indentGuidesStroke",
      all(ref(activeIndentGuide) |> transparent(0.5)),
    );

  let filterMatchBackground =
    define("list.filterMatchBackground", all(ref(hoverBackground)));

  let filterMatchBorder =
    define("list.filterMatchBorder", all(ref(hoverForeground)));

  let defaults = [
    focusBackground,
    focusForeground,
    focusOutline,
    activeSelectionBackground,
    activeSelectionForeground,
    hoverBackground,
    hoverForeground,
    highlightForeground,
    deemphasizedForeground,
    activeIndentGuide,
    inactiveIndentGuide,
    inactiveFocusOutline,
    filterMatchBackground,
    filterMatchBorder,
  ];
};

module EditorSuggestWidget = {
  let background =
    define(
      "editorSuggestWidget.background",
      all(ref(EditorWidget.background)),
    );
  let border =
    define("editorSuggestWidget.border", all(ref(EditorWidget.border)));
  let foreground =
    define("editorSuggestWidget.foreground", all(ref(Editor.foreground)));
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
    foreground,
    highlightForeground,
    selectedBackground,
  ];
};

module Selection = {
  let background =
    define("selection.background", all(ref(Editor.selectionBackground))); // actually: unspecified

  let defaults = [background];
};

module Input = {
  let background =
    define(
      "input.background",
      {dark: hex("#3C3C3C"), light: hex("#FFF"), hc: hex("#000")},
    );
  let foreground = define("input.foreground", all(ref(foreground)));
  let border =
    define(
      "input.border",
      {dark: unspecified, light: unspecified, hc: ref(contrastBorder)},
    );
  let placeholderForeground =
    define(
      "input.placeholderForeground",
      {
        dark: ref(foreground) |> transparent(0.5),
        light: ref(foreground) |> transparent(0.5),
        hc: ref(foreground) |> transparent(0.7),
      },
    );

  let defaults = [background, foreground, border, placeholderForeground];
};

module Menu = {
  let background = define("menu.background", all(ref(Dropdown.background)));
  let foreground =
    define(
      "menu.foreground",
      {
        dark: ref(Dropdown.foreground),
        light: ref(foreground),
        hc: ref(foreground),
      },
    );
  let selectionBackground =
    define(
      "menu.selectionBackground",
      all(ref(List.activeSelectionBackground)),
    );

  let defaults = [background, foreground, selectionBackground];
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
    | Select(_)
    | TerminalVisual(_)
    | Visual(_) => visualModeBackground
    | CommandLine => commandlineModeBackground
    | Operator(_) => operatorModeBackground
    | Snippet
    | TerminalInsert
    | Insert(_) => insertModeBackground
    | Replace(_) => replaceModeBackground
    | TerminalNormal
    | Normal(_) => normalModeBackground
    };

  let foregroundFor = (mode: Mode.t) =>
    switch (mode) {
    | Select(_)
    | TerminalVisual(_)
    | Visual(_) => visualModeForeground
    | CommandLine => commandlineModeForeground
    | Operator(_) => operatorModeForeground
    | Snippet
    | TerminalInsert
    | Insert(_) => insertModeForeground
    | Replace(_) => replaceModeForeground
    | TerminalNormal
    | Normal(_) => normalModeForeground
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

  module Modal = {
    let backdrop = define("oni.modal.backdrop", all(hex("#0004")));
    let background =
      define("oni.modal.background", all(ref(Notifications.background)));
    let foreground =
      define("oni.modal.foreground", all(ref(Notifications.foreground)));
    let border = define("oni.modal.border", all(ref(Notifications.border)));
    let shortcutForeground =
      define(
        "oni.modal.shortcutForeground",
        all(ref(foreground) |> transparent(0.7)),
      );
    let shortcutHighlightForeground =
      define(
        "oni.modal.shortcutHighlightForeground",
        all(ref(normalModeBackground)),
      );

    let defaults = [
      backdrop,
      background,
      foreground,
      shortcutForeground,
      shortcutHighlightForeground,
    ];
  };

  module Sneak = {
    let background =
      define("oni.sneak.background", all(ref(Menu.selectionBackground)));
    let foreground =
      define("oni.sneak.foreground", all(ref(Menu.foreground)));
    let highlight =
      define("oni.sneak.highlight", all(ref(normalModeBackground)));

    let defaults = [background, foreground, highlight];
  };
};

module Panel = {
  let background = define("panel.background", all(ref(Editor.background)));
  let border =
    define(
      "panel.border",
      {
        dark: hex("#808080") |> transparent(0.35),
        light: hex("#808080") |> transparent(0.35),
        hc: ref(contrastBorder),
      },
    );

  let defaults = [background, border];
};

module PanelTitle = {
  let activeForeground =
    define(
      "panelTitle.activeForeground",
      {dark: hex("#E7E7E7"), light: hex("#424242"), hc: hex("#FFF")},
    );

  let inactiveForeground =
    define(
      "panelTitle.inactiveForeground",
      {
        dark: ref(activeForeground) |> transparent(0.6),
        light: ref(activeForeground) |> transparent(0.71),
        hc: hex("#FFF"),
      },
    );

  let activeBorder =
    define(
      "panelTitle.activeBorder",
      {
        dark: ref(activeForeground),
        light: ref(activeForeground),
        hc: ref(contrastBorder),
      },
    );

  let dropBackground =
    define(
      "panelTitle.dropBackground",
      {
        dark: hex("#FFF") |> transparent(0.12),
        light: hex("#2677CB") |> transparent(0.18),
        hc: hex("#FFF") |> transparent(0.12),
      },
    );

  let defaults = [
    activeForeground,
    inactiveForeground,
    activeBorder,
    dropBackground,
  ];
};

module PanelInput = {
  let border =
    define(
      "panelInput.border",
      {
        dark: ref(Input.border), // actually: unspecified
        light: hex("#ddd"),
        hc: unspecified,
      },
    );

  let defaults = [border];
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

module Minimap = {
  let findMatchHighlight =
    define(
      "minimap.findMatchHighlight",
      {light: hex("#d18616"), dark: hex("#d18616"), hc: hex("#AB5A00")},
    );
  let selectionHighlight =
    define(
      "minimap.selectionHighlight",
      {light: hex("#ADD6FF"), dark: hex("#264F78"), hc: hex("#ffffff")},
    );
  let errorHighlight =
    define(
      "minimap.errorHighlight",
      {
        dark: color(Revery.Color.rgb_int(255, 18, 18)) |> transparent(0.7),
        light: color(Revery.Color.rgb_int(255, 18, 18)) |> transparent(0.7),
        hc: color(Revery.Color.rgb_int(255, 50, 50)),
      },
    );
  let warningHighlight =
    define("minimap.warningHighlight", all(ref(EditorWarning.foreground)));
  let background =
    define("minimap.background", all(ref(Editor.background)));

  let defaults = [
    findMatchHighlight,
    selectionHighlight,
    errorHighlight,
    warningHighlight,
    background,
  ];
};

module MinimapSlider = {
  let background =
    define(
      "minimapSlider.background",
      all(ref(ScrollbarSlider.background) |> transparent(0.5)),
    );
  let hoverBackground =
    define(
      "minimapSlider.hoverBackground",
      all(ref(ScrollbarSlider.hoverBackground) |> transparent(0.5)),
    );
  let activeBackground =
    define(
      "minimapSlider.activeBackground",
      all(ref(ScrollbarSlider.activeBackground) |> transparent(0.5)),
    );

  let defaults = [background, hoverBackground, activeBackground];
};

module MinimapGutter = {
  let addedBackground =
    define(
      "minimapGutter.addedBackground",
      {
        dark: color(Revery.Color.rgb_int(12, 125, 157)),
        light: color(Revery.Color.rgb_int(102, 175, 224)),
        hc: color(Revery.Color.rgb_int(0, 155, 249)),
      },
    );
  let modifiedBackground =
    define(
      "minimapGutter.modifiedBackground",
      {
        dark: color(Revery.Color.rgb_int(88, 124, 12)),
        light: color(Revery.Color.rgb_int(129, 184, 139)),
        hc: color(Revery.Color.rgb_int(51, 171, 78)),
      },
    );
  let deletedBackground =
    define(
      "minimapGutter.deletedBackground",
      {
        dark: color(Revery.Color.rgb_int(148, 21, 27)),
        light: color(Revery.Color.rgb_int(202, 75, 81)),
        hc: color(Revery.Color.rgb_int(252, 93, 109)),
      },
    );

  let defaults = [addedBackground, modifiedBackground, deletedBackground];
};

module SideBar = {
  let background =
    define(
      "sideBar.background",
      {dark: hex("#252526"), light: hex("#F3F3F3"), hc: hex("#000")},
    );
  let foreground = define("sideBar.foreground", all(ref(foreground))); // actually: all(unspecified)

  let defaults = [background, foreground];
};

module SideBarSectionHeader = {
  let background =
    define(
      "sideBarSectionHeader.background",
      {
        dark: hex("#808080") |> transparent(0.2),
        light: hex("#808080") |> transparent(0.2),
        hc: unspecified,
      },
    );

  let foreground =
    define(
      "sideBarSectionHeader.foreground",
      {
        dark: ref(SideBar.foreground),
        light: ref(SideBar.foreground),
        hc: ref(SideBar.foreground),
      },
    );

  let border = contrastBorder;

  let defaults = [background, foreground, border];
};

module StatusBar = {
  let background =
    define(
      "statusBar.background",
      {dark: hex("#000000AA"), light: hex("#FFFFFFAA"), hc: unspecified},
    );
  let foreground = define("statusBar.foreground", all(hex("#FFF")));

  let errorItemBackground =
    define("statusBarItem.errorBackground", all(hex("#FF0000FF")));

  let errorItemForeground =
    define("statusBarItem.errorForeground", all(ref(foreground)));

  let defaults = [
    background,
    foreground,
    errorItemBackground,
    errorItemForeground,
  ];
};

module SymbolIcon = {
  let arrayForeground =
    define("symbolIcon.arrayForeground", all(ref(foreground)));
  let booleanForeground =
    define("symbolIcon.booleanForeground", all(ref(foreground)));
  let classForeground =
    define(
      "symbolIcon.classForeground",
      {dark: hex("#EE9D28"), light: hex("#D67E00"), hc: hex("#EE9D28")},
    );
  let colorForeground =
    define("symbolIcon.colorForeground", all(ref(foreground)));
  let constantForeground =
    define("symbolIcon.constantForeground", all(ref(foreground)));
  let constructorForeground =
    define(
      "symbolIcon.constructorForeground",
      {dark: hex("#B180D7"), light: hex("#652D90"), hc: hex("#B180D7")},
    );
  let enumeratorForeground =
    define(
      "symbolIcon.enumeratorForeground",
      {dark: hex("#EE9D28"), light: hex("#D67E00"), hc: hex("#EE9D28")},
    );
  let enumeratorMemberForeground =
    define(
      "symbolIcon.enumeratorMemberForeground",
      {dark: hex("#75BEFF"), light: hex("#007ACC"), hc: hex("#75BEFF")},
    );
  let eventForeground =
    define(
      "symbolIcon.eventForeground",
      {dark: hex("#EE9D28"), light: hex("#D67E00"), hc: hex("#EE9D28")},
    );
  let fieldForeground =
    define(
      "symbolIcon.fieldForeground",
      {dark: hex("#75BEFF"), light: hex("#007ACC"), hc: hex("#75BEFF")},
    );
  let fileForeground =
    define("symbolIcon.fileForeground", all(ref(foreground)));
  let folderForeground =
    define("symbolIcon.folderForeground", all(ref(foreground)));
  let functionForeground =
    define(
      "symbolIcon.functionForeground",
      {dark: hex("#B180D7"), light: hex("#652D90"), hc: hex("#B180D7")},
    );
  let interfaceForeground =
    define(
      "symbolIcon.interfaceForeground",
      {dark: hex("#75BEFF"), light: hex("#007ACC"), hc: hex("#75BEFF")},
    );
  let keyForeground =
    define("symbolIcon.keyForeground", all(ref(foreground)));
  let keywordForeground =
    define("symbolIcon.keywordForeground", all(ref(foreground)));
  let methodForeground =
    define(
      "symbolIcon.methodForeground",
      {dark: hex("#B180D7"), light: hex("#652D90"), hc: hex("#B180D7")},
    );
  let moduleForeground =
    define("symbolIcon.moduleForeground", all(ref(foreground)));
  let namespaceForeground =
    define("symbolIcon.namespaceForeground", all(ref(foreground)));
  let nullForeground =
    define("symbolIcon.nullForeground", all(ref(foreground)));
  let objectForeground =
    define("symbolIcon.objectForeground", all(ref(foreground)));
  let operatorForeground =
    define("symbolIcon.operatorForeground", all(ref(foreground)));
  let packageForeground =
    define("symbolIcon.packageForeground", all(ref(foreground)));
  let propertyForeground =
    define("symbolIcon.propertyForeground", all(ref(foreground)));
  let referenceForeground =
    define("symbolIcon.referenceForeground", all(ref(foreground)));
  let snippetForeground =
    define("symbolIcon.snippetForeground", all(ref(foreground)));
  let stringForeground =
    define("symbolIcon.stringForeground", all(ref(foreground)));
  let structForeground =
    define("symbolIcon.structForeground", all(ref(foreground)));
  let textForeground =
    define("symbolIcon.textForeground", all(ref(foreground)));
  let typeParameterForeground =
    define("symbolIcon.typeParameterForeground", all(ref(foreground)));
  let unitForeground =
    define("symbolIcon.unitForeground", all(ref(foreground)));
  let variableForeground =
    define(
      "symbolIcon.variableForeground",
      {dark: hex("#75BEFF"), light: hex("#007ACC"), hc: hex("#75BEFF")},
    );

  let defaults = [
    arrayForeground,
    booleanForeground,
    classForeground,
    colorForeground,
    constantForeground,
    constructorForeground,
    enumeratorForeground,
    enumeratorMemberForeground,
    eventForeground,
    fieldForeground,
    fileForeground,
    folderForeground,
    functionForeground,
    interfaceForeground,
    keyForeground,
    keywordForeground,
    methodForeground,
    moduleForeground,
    namespaceForeground,
    nullForeground,
    objectForeground,
    packageForeground,
    propertyForeground,
    referenceForeground,
    snippetForeground,
    stringForeground,
    structForeground,
    textForeground,
    typeParameterForeground,
    unitForeground,
    variableForeground,
  ];
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

module Terminal = {
  open Revery;
  let background =
    define("terminal.background", color(Color.rgb_int(0, 0, 0)) |> all);
  let foreground =
    define(
      "terminal.foreground",
      color(Color.rgb_int(233, 235, 235)) |> all,
    );
  let ansiBlack =
    define("terminal.ansiBlack", color(Color.rgb_int(0, 0, 0)) |> all);
  let ansiRed =
    define("terminal.ansiRed", color(Color.rgb_int(194, 54, 33)) |> all);
  let ansiGreen =
    define("terminal.ansiGreen", color(Color.rgb_int(37, 188, 36)) |> all);
  let ansiYellow =
    define(
      "terminal.ansiYellow",
      color(Color.rgb_int(173, 173, 39)) |> all,
    );
  let ansiBlue =
    define("terminal.ansiBlue", color(Color.rgb_int(73, 46, 225)) |> all);
  let ansiMagenta =
    define(
      "terminal.ansiMagenta",
      color(Color.rgb_int(211, 56, 211)) |> all,
    );
  let ansiCyan =
    define("terminal.ansiCyan", color(Color.rgb_int(51, 197, 200)) |> all);
  let ansiWhite =
    define(
      "terminal.ansiWhite",
      color(Color.rgb_int(203, 204, 205)) |> all,
    );
  let ansiBrightBlack =
    define(
      "terminal.ansiBrightBlack",
      color(Color.rgb_int(129, 131, 131)) |> all,
    );
  let ansiBrightRed =
    define(
      "terminal.ansiBrightRed",
      color(Color.rgb_int(252, 57, 31)) |> all,
    );
  let ansiBrightGreen =
    define(
      "terminal.ansiBrightGreen",
      color(Color.rgb_int(49, 231, 34)) |> all,
    );
  let ansiBrightYellow =
    define(
      "terminal.ansiBrightYellow",
      color(Color.rgb_int(234, 236, 35)) |> all,
    );
  let ansiBrightBlue =
    define(
      "terminal.ansiBrightBlue",
      color(Color.rgb_int(88, 51, 255)) |> all,
    );
  let ansiBrightMagenta =
    define(
      "terminal.ansiBrightMagenta",
      color(Color.rgb_int(20, 240, 240)) |> all,
    );
  let ansiBrightCyan =
    define(
      "terminal.ansiBrightCyan",
      color(Color.rgb_int(20, 240, 240)) |> all,
    );
  let ansiBrightWhite =
    define(
      "terminal.ansiBrightWhite",
      color(Color.rgb_int(233, 235, 235)) |> all,
    );

  let defaults = [
    background,
    foreground,
    ansiBlack,
    ansiRed,
    ansiGreen,
    ansiYellow,
    ansiBlue,
    ansiMagenta,
    ansiCyan,
    ansiWhite,
    ansiBrightBlack,
    ansiBrightRed,
    ansiBrightGreen,
    ansiBrightYellow,
    ansiBrightBlue,
    ansiBrightMagenta,
    ansiBrightCyan,
    ansiBrightWhite,
  ];
};

module TextLink = {
  let foreground =
    define(
      "textLink.foreground",
      {dark: hex("#3794FF"), light: hex("#006AB1"), hc: hex("#3794FF")},
    );

  let activeForeground =
    define(
      "textLink.activeForeground",
      {dark: hex("#3794FF"), light: hex("#006AB1"), hc: hex("#3794FF")},
    );

  let defaults = [foreground, activeForeground];
};

module TitleBar = {
  let activeForeground =
    define(
      "titleBar.activeForeground",
      {dark: hex("#CCC"), light: hex("#333"), hc: hex("#FFF")},
    );
  let inactiveForeground =
    define(
      "titleBar.inactiveForeground",
      {
        dark: ref(activeForeground) |> transparent(0.6),
        light: ref(activeForeground) |> transparent(0.6),
        hc: unspecified,
      },
    );
  let activeBackground =
    define(
      "titleBar.activeBackground",
      {dark: hex("#3C3C3C"), light: hex("#DDD"), hc: hex("#000")},
    );
  let inactiveBackground =
    define(
      "titleBar.inactiveBackground",
      {
        dark: ref(activeBackground) |> transparent(0.6),
        light: ref(activeBackground) |> transparent(0.6),
        hc: unspecified,
      },
    );
  let border =
    define(
      "titleBar.border",
      {dark: unspecified, light: unspecified, hc: ref(contrastBorder)},
    );

  let hoverBackground =
    define(
      "oni.titleBar.hoverBackground",
      {
        dark: hex("#FFFFFF") |> transparent(0.1),
        light: hex("#000000") |> transparent(0.1),
        hc: unspecified,
      },
    );

  let hoverCloseBackground =
    define(
      "oni.titleBar.hoverCloseBackground",
      hex("#E81123") |> transparent(0.9) |> all,
    );

  let defaults = [
    activeForeground,
    inactiveForeground,
    activeBackground,
    inactiveBackground,
    border,
    hoverBackground,
    hoverCloseBackground,
  ];
};

let defaults = [foreground, contrastBorder, shadow];
