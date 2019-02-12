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
        2
    } else {
        float_of_int(lines)
        |> log10
        |> v => v +. epsilon_float +. 1.0
        |> int_of_float
        |> v => max(v, 2)
    }
};

let getLineNumberPixelWidth = (~lines: int, ~fontPixelWidth: int, ()) => {
    let digits = getNumberOfDigitsForLines(lines);   

    /* Add some padding around the line number */
    (digits + 2) * fontPixelWidth
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
