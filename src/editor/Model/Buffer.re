/*
 * Buffer.re
 *
 * In-memory text buffer representation
 */

open Oni_Core;
open Oni_Core.Types;

type t = {
  metadata: Vim.BufferMetadata.t,
  lines: array(string),
};

let show = (v: t) => "TODO";

let ofLines = (lines: array(string)) => {
  metadata: Vim.BufferMetadata.create(),
  lines,
};

let empty = ofLines([||]);

let ofMetadata = (metadata: Vim.BufferMetadata.t) => {metadata, lines: [||]};

let getMetadata = (buffer: t) => buffer.metadata;

let getId = (buffer: t) => buffer.metadata.id;

let getLine = (buffer: t, line: int) => buffer.lines[line];

let getUri = (buffer: t) => {
  let getUriFromMetadata = (metadata: Vim.BufferMetadata.t) => {
    switch (metadata.filePath) {
    | None => Uri.fromMemory(string_of_int(metadata.id))
    | Some(v) => Uri.fromPath(v)
    };
  };

  buffer |> getMetadata |> getUriFromMetadata;
};

/*
 * TODO:
 * - Handle variable tab sizes, based on indentation settings
 * - Handle multibyte characters
 */
let getLineLength = (buffer: t, line: int) => {
  let line = getLine(buffer, line);
  String.length(line);
};

let getNumberOfLines = (buffer: t) => Array.length(buffer.lines);

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
  let startLine = update.startLine |> Index.toZeroBasedInt;
  let endLine = update.endLine |> Index.toZeroBasedInt;
  if (Array.length(lines) == 0) {
    updateLines;
  } else if (startLine >= Array.length(lines)) {
    let ret = Array.concat([lines, updateLines]);
    ret;
  } else {
    let prev = slice(~lines, ~start=0, ~length=startLine, ());
    let post =
      slice(
        ~lines,
        ~start=endLine,
        ~length=Array.length(lines) - endLine,
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
  | {startLine: ZeroBasedIndex(0), endLine: ZeroBasedIndex((-1)), version, _} => {
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

let updateMetadata = (metadata: Vim.BufferMetadata.t, buf: t) => {
  {...buf, metadata};
};
