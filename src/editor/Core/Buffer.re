/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

open Types;

[@deriving show]
type t = {
  metadata: BufferMetadata.t,
  lines: array(string),
};

let ofLines = (lines: array(string)) => {
  metadata: BufferMetadata.create(),
  lines,
};

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
  switch (update) {
  /***
     When a buffer is first attached it emits an update with
     a startLine of 0 and endLine of -1 in this case we should
     update the buffer's version but set the content of the buffer
     rather than update it, which would result in duplication
   */
  | {startLine: 0, endLine: (-1), version, _} => {
      metadata: {
        ...buf.metadata,
        version,
      },
      lines: Array.of_list(update.lines),
    }
  | {version, _} when version > buf.metadata.version =>
    let metadata = {...buf.metadata, version: update.version};
    {metadata, lines: applyUpdate(buf.lines, update)};
  | _ => buf
  };
