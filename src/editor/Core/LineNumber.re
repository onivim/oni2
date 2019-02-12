/*
 * LineNumber.re
 *
 * Settings and utilities for managing Linumbers
 */

open Types;

type setting =
| On
| Off
| Relative

/*
 * Given a number of lines, gives the number of digits
 * needed to represent the line numbers
 */
let getNumberOfDigitsForLines = (lines: int) => {
    if (lines <= 0) {
        1
    } else {
        float_of_int(lines)
        |> log10
        |> v => v +. epsilon_float +. 1.0
        |> int_of_float
    }
};

let getLineNumber = (~bufferLine: Index.t, ~cursorLine: Index.t, ~setting: setting, ()) => {
    switch (setting) {
    | Relative => {

        let oneBasedBufferLine = Index.toOneBasedInt(bufferLine);
        let oneBasedCursorLine = Index.toOneBasedInt(cursorLine);

        if (oneBasedBufferLine === oneBasedCursorLine) {
            oneBasedBufferLine
        } else {
            abs(oneBasedCursorLine - oneBasedBufferLine)
        }
    }
    | _ => Index.toOneBasedInt(bufferLine)
    }
}
