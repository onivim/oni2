/*
 * Constants.re
 *
 * Constants used across Oni2. Some of these may make sense
 * to pull out to configuration values
 */

type t = {
  /* Horizontal padding on each side of the minimap */
  minimapPadding: int,
  /*
   * Width of characters in minimap, in pixels
   */
  minimapCharacterWidth: int,
  /*
   * Height of characters in minimap, in pixels
   */
  minimapCharacterHeight: int,
  /*
   * Number of pixels between each line in the minimap
   */
  minimapLineSpacing: int,
  scrollBarThickness: int,
  minimapMaxColumn: int,
  /*
   * Maximum levels of the file system to traverse
   * when initially populating the file explorer
   */
  maximumExplorerDepth: int,
  tabHeight: int,

  /*
   * The line count considered a 'large file' - if a file exceeds this limit,
   * some features like syntax highlighting will be disabled.
   */
  largeFileLineCountThreshold: int,
};

let default: t = {
  minimapPadding: 0,
  minimapCharacterWidth: 1,
  minimapCharacterHeight: 2,
  minimapLineSpacing: 1,
  scrollBarThickness: 15,
  minimapMaxColumn: 120,
  maximumExplorerDepth: 1,
  tabHeight: 35,

  /*
   * The threshold we set right now is artificially low,
   * because our current textmate highlighting strategy is very slow.
   * We'll switch to a native strategy, and bump this up.
   */
  largeFileLineCountThreshold: 1000,
};
