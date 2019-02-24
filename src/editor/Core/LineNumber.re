/*
 * LineNumber.re
 *
 * Settings and utilities for managing Linumbers
 */

[@deriving yojson]
type setting =
  | [@name "on"] On
  | [@name "off"] Off
  | [@name "relative"] Relative;

/*
 * Given a number of lines, gives the number of digits
 * needed to represent the line numbers
 */
let getNumberOfDigitsForLines = (lines: int) =>
  if (lines <= 0) {
    2;
  } else {
    float_of_int(lines)
    |> log10
    |> (v => v +. epsilon_float +. 1.0 |> int_of_float |> (v => max(v, 2)));
  };

let getLineNumberPixelWidth = (~lines: int, ~fontPixelWidth: int, ()) => {
  let digits = getNumberOfDigitsForLines(lines);

  /* Add some padding around the line number */
  (digits + 2) * fontPixelWidth;
};

let getLineNumber =
    (~bufferLine: int, ~cursorLine: int, ~setting: setting, ()) =>
  switch (setting) {
  | Relative =>
    if (bufferLine === cursorLine) {
      bufferLine;
    } else {
      abs(bufferLine - cursorLine);
    }
  | _ => bufferLine
  };
