/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

open Types;

type t = {
  file: option(string),
  lines: array(string),
};

let ofLines = (lines: array(string)) => {
  let ret: t = {file: None, lines};
  ret;
};

let slice = (~lines: array(string), ~start, ~length, ()) => {
  let len = Array.length(lines);
  if (start >= len) {
    [||];
  } else {
    let len = min(start + length, len) - start;
    if (len <= 0) {
      [||];
    } else {
      Array.sub(lines, start, len);
    };
  };
};

let applyUpdate = (lines: array(string), update: BufferUpdate.t) => {

    if (update.endLine <= update.startLine) {
        Array.of_list(update.lines)
    } else {

  let prev = slice(~lines, ~start=0, ~length=update.startLine, ());
  let post =
    slice(
      ~lines,
      ~start=update.endLine,
      ~length=Array.length(lines) - update.endLine,
      (),
    );

  let lines = Array.of_list(update.lines);

  Array.concat([prev, lines, post]);
    }
};

let update = (buf: t, update: BufferUpdate.t) => {
  let ret: t = {...buf, lines: applyUpdate(buf.lines, update)};
  ret;
};
