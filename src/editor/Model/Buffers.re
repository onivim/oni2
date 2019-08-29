/*
 * Buffers.re
 *
 * A collection of buffers
 */

open Oni_Core;

type t = IntMap.t(Buffer.t);

let empty = IntMap.empty;

type mapFunction = Buffer.t => Buffer.t;

let map = IntMap.map;
let update = IntMap.update;
let remove = IntMap.remove;

let log = (m: t) =>
  IntMap.iter(
    (_, b) =>
      Buffer.show(b) |> (++)("Buffer ======================: \n") |> Log.info,
    m,
  );

let getBuffer = (id, map) => IntMap.find_opt(id, map);

let anyModified = (buffers: t) => {
  IntMap.fold(
    (_key, v, prev) => Buffer.isModified(v) || prev,
    buffers,
    false,
  );
};

let ofBufferOpt = (f, buffer) => {
  switch (buffer) {
  | None => None
  | Some(b) => Some(f(b))
  };
};

let applyBufferUpdate = bufferUpdate =>
  ofBufferOpt(buffer => Buffer.update(buffer, bufferUpdate));

let setIndentation = indent =>
  ofBufferOpt(buffer => Buffer.setIndentation(indent, buffer));

let disableSyntaxHighlighting =
  ofBufferOpt(buffer => Buffer.disableSyntaxHighlighting(buffer));

let setModified = modified =>
  ofBufferOpt(buffer => Buffer.setModified(modified, buffer));

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | BufferDisableSyntaxHighlighting(id) =>
    IntMap.update(id, disableSyntaxHighlighting, state)
  | BufferEnter(metadata, fileType) =>
    let f = buffer =>
      switch (buffer) {
      | Some(v) =>
        Some(
          v
          |> Buffer.setModified(metadata.modified)
          |> Buffer.setFilePath(metadata.filePath)
          |> Buffer.setVersion(metadata.version)
        )
      | None => Some(
          Buffer.ofMetadata(metadata)
          |> Buffer.setFileType(fileType)
        )
      };
    IntMap.update(metadata.id, f, state);
  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | BufferSetModified(id, modified) =>
    IntMap.update(id, setModified(modified), state)
  | BufferSetIndentation(id, indent) =>
    IntMap.update(id, setIndentation(indent), state)
  | BufferUpdate(bu) => IntMap.update(bu.id, applyBufferUpdate(bu), state)
  | BufferSaved(metadata) =>
    IntMap.update(metadata.id, setModified(metadata.modified), state)
  | _ => state
  };
};
