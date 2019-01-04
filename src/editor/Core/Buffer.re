/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

type t = {
    file: option(string),
    lines: array(string),
};

let ofLines = (lines: array(string)) => {
    let ret: t = {
        file: None,
        lines,
    };
    ret;
}
