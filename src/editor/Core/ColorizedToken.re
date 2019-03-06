/*
 * ColorizedToken.re
 */

/* From:
 * https://github.com/Microsoft/vscode-textmate/blob/master/src/main.ts
 */
let languageId_mask = 0b00000000000000000000000011111111;
let token_type_mask = 0b00000000000000000000011100000000;
let font_style_mask = 0b00000000000000000011100000000000;
let foreground_mask = 0b00000000011111111100000000000000;
let background_mask = 0b11111111100000000000000000000000;

let languageid_offset = 0;
let token_type_offset = 8;
let font_style_offset = 11;
let foreground_offset = 14;
let background_offset = 23;

[@deriving show]
type t = {
  index: int,
  foregroundColor: int,
  backgroundColor: int,
};

let getForegroundColor: int => int =
  v => {
    (v land foreground_mask) lsr foreground_offset;
  };

let getBackgroundColor: int => int =
  v => {
    (v land background_mask) lsr background_offset;
  };

let create: (int, int) => t =
  (idx, v) => {
    index: idx,
    foregroundColor: getForegroundColor(v) - 1,
    backgroundColor: getBackgroundColor(v) - 1,
  };

let ofColors: (int, int, int) => t =
  (idx, foregroundColor, backgroundColor) => {
    index: idx,
    foregroundColor,
    backgroundColor,
  };

let default: t = {index: 0, foregroundColor: 0, backgroundColor: 1};
