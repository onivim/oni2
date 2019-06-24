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
      Buffer.show(b)
      |> (++)("Buffer ======================: \n")
      |> print_endline,
    m,
  );

let getBuffer = (id, map) => IntMap.find_opt(id, map);

let getBufferMetadata = (id, map) =>
  switch (getBuffer(id, map)) {
  | Some(v) => Some(Buffer.getMetadata(v))
  | None => None
  };

let applyBufferUpdate = (bufferUpdate, buffer) =>
  switch (buffer) {
  | None => None
  | Some(b) => Some(Buffer.update(b, bufferUpdate))
  };

let updateMetadata = (metadata, buffer) => {
  switch (buffer) {
  | None => None
  | Some(b) => Some(b |> Buffer.updateMetadata(metadata))
  };
};

let setIndentation = (indent, buffer) => {
  switch (buffer) {
  | None => None
  | Some(b) => Some(Buffer.setIndentation(indent, b))
  };
};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
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
