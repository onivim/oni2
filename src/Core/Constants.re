/*
 * Constants.re
 *
 * Constants used across Oni2. Some of these may make sense
 * to pull out to configuration values
 */

let minimumFontSize = 6.;
let defaultFontSize = 14.;

let defaultFontFile = "JetBrainsMono-Regular.ttf";

let isDefaultFont = str => {
  // Before we switched to JetBrains Mono as the default font...
  // "FiraCode-Regular.ttf" was specified in the default configuration file.
  // So if we see it again, we should treat it as a default font (#3208)
  String.equal(str, "FiraCode-Regular.ttf")
  || String.equal(str, defaultFontFile);
};

let defaultFontFamily =
  Revery.Font.Family.fromFiles((~weight, ~italic) => {
    switch (weight, italic) {
    | (Revery.Font.Weight.Thin, false)
    | (Revery.Font.Weight.UltraLight, false) => "JetBrainsMono-ExtraLight.ttf"

    | (Revery.Font.Weight.Thin, true)
    | (Revery.Font.Weight.UltraLight, true) => "JetBrainsMono-ExtraLight-Italic.ttf"

    | (Revery.Font.Weight.Light, true) => "JetBrainsMono-Light-Italic.ttf"
    | (Revery.Font.Weight.Light, false) => "JetBrainsMono-Light.ttf"

    | (Revery.Font.Weight.Undefined, true)
    | (Revery.Font.Weight.Normal, true) => "JetBrainsMono-Italic.ttf"

    | (Revery.Font.Weight.Undefined, false)
    | (Revery.Font.Weight.Normal, false) => defaultFontFile

    | (Revery.Font.Weight.Medium, true)
    | (Revery.Font.Weight.SemiBold, true) => "JetBrainsMono-Medium-Italic.ttf"

    | (Revery.Font.Weight.Medium, false)
    | (Revery.Font.Weight.SemiBold, false) => "JetBrainsMono-Medium.ttf"

    | (Revery.Font.Weight.Bold, true) => "JetBrainsMono-Bold-Italic.ttf"
    | (Revery.Font.Weight.Bold, false) => "JetBrainsMono-Bold.ttf"

    | (Revery.Font.Weight.UltraBold, true)
    | (Revery.Font.Weight.Heavy, true) => "JetBrainsMono-ExtraBold-Italic.ttf"

    | (Revery.Font.Weight.UltraBold, false)
    | (Revery.Font.Weight.Heavy, false) => "JetBrainsMono-ExtraBold.ttf"
    }
  });
let defaultTerminalFontSize = 12.;

let defaultTheme = "LaserWave Italic";

let diffMarkersMaxLineCount = 2000;

let syntaxEagerMaxLines = 500;
let syntaxEagerMaxLineLength = 1000;
let syntaxEagerBudget = 0.25; /* 250 milliseconds */

/* Horizontal padding on each side of the minimap */
let minimapPadding = 0;

/*
 * Width of characters in minimap, in pixels
 */
let minimapCharacterWidth = 1;
/*
 * Height of characters in minimap, in pixels
 */
let minimapCharacterHeight = 2;
/*
 * Number of pixels between each line in the minimap
 */
let minimapLineSpacing = 1;
let scrollBarCursorSize = 2;
let minimapMaxColumn = 120;
let tabHeight = 35;
let notificationWidth = 300;

/*
 * The line count considered a 'large file' - if a file exceeds this limit,
 * some features like syntax highlighting will be disabled.
 * The threshold we set right now is artificially low,
 * because our current textmate highlighting strategy is very slow.
 * We'll switch to a native strategy, and bump this up.
 */
let largeFileLineCountThreshold = 300 * 1000;

let doubleClickTime = Revery.Time.milliseconds(500);

// Number of pixels the mouse needs to be within the border in order to trigger auto-scroll.
let mouseAutoScrollBorder = 75;
let mouseAutoScrollSpeed = 75.;

let mouseAutoScrollInterval = Revery.Time.milliseconds(50);

let highPriorityDebounceTime = Revery.Time.milliseconds(50);
let mediumPriorityDebounceTime = Revery.Time.milliseconds(100);
let lowPriorityDebounceTime = Revery.Time.milliseconds(500);
