/*
 * LineNumber.re
 *
 * Settings and utilities for managing Linumbers
 */

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

