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

let getBufferMetadata = (id, map) =>
  switch (getBuffer(id, map)) {
  | None => None
  | Some(v) => Some(Buffer.getMetadata(v))
  };

let applyBufferUpdate = bufferUpdate =>
  ofBufferOpt(buffer => Buffer.update(buffer, bufferUpdate));

let updateMetadata = metadata =>
  ofBufferOpt(buffer => Buffer.updateMetadata(metadata, buffer));

let setIndentation = indent =>
  ofBufferOpt(buffer => Buffer.setIndentation(indent, buffer));

let disableSyntaxHighlighting =
  ofBufferOpt(buffer => Buffer.disableSyntaxHighlighting(buffer));

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | BufferDisableSyntaxHighlighting(id) =>
    IntMap.update(id, disableSyntaxHighlighting, state)
  | BufferEnter(metadata) =>
    let f = buffer =>
      switch (buffer) {
      | Some(v) => Some(v |> Buffer.updateMetadata(metadata))
      | None => Some(Buffer.ofMetadata(metadata))
      };
    IntMap.update(metadata.id, f, state);
  /* | BufferDelete(bd) => IntMap.remove(bd, state) */
  | BufferSetIndentation(id, indent) =>
    IntMap.update(id, setIndentation(indent), state)
  | BufferUpdate(bu) => IntMap.update(bu.id, applyBufferUpdate(bu), state)
  | BufferSaved(metadata) =>
    IntMap.update(metadata.id, updateMetadata(metadata), state)
  | _ => state
  };
};
