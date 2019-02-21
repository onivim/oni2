/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

open Types;

type t = {
  metadata: BufferMetadata.t,
  lines: array(string),
};

let ofLines = (lines: array(string)) => {
  metadata: BufferMetadata.create(),
  lines,
};

let show = (b: t) =>
  "Buffer ["
  ++ string_of_int(b.metadata.id)
  ++ "]: "
  ++ String.concat("\n", Array.to_list(b.lines));

let ofMetadata = (metadata: BufferMetadata.t) => {metadata, lines: [||]};

let slice = (~lines: array(string), ~start, ~length, ()) => {
  let len = Array.length(lines);
  if (start >= len) {
    [||];
  } else {
    let start = max(start, 0);
    let len = min(start + length, len) - start;
    if (len <= 0) {
      [||];
    } else {
      Array.sub(lines, start, len);
    };
  };
};

let applyUpdate = (lines: array(string), update: BufferUpdate.t) => {
  let updateLines = Array.of_list(update.lines);
  if (Array.length(lines) == 0) {
    updateLines;
  } else if (update.startLine >= Array.length(lines)) {
    let ret = Array.concat([lines, updateLines]);
    ret;
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
  };
};

let update = (buf: t, update: BufferUpdate.t) =>
  if (update.version > buf.metadata.version) {
    let metadata = {...buf.metadata, modified: true, version: update.version};

    {metadata, lines: applyUpdate(buf.lines, update)};
  } else {
    buf;
  };
