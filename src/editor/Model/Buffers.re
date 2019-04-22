/*
 * Buffers.re
 *
 * A collection of buffers
 */

open Oni_Core;
open Oni_Core.Types;

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

let markSaved = buffer => {
  switch (buffer) {
  | None => None
  | Some(b) => Some(Buffer.markSaved(b))
  };
};

let reduce = (state: t, action: Actions.t) => {
  print_endline("BUFFERS: REDUCE");
  switch (action) {
  | BufferEnter(id) =>
    let f = buffer =>
      switch (buffer) {
      | Some(v) => Some(v)
      | None => Some(Buffer.ofMetadata(BufferMetadata.create(~id, ())))
      };
    IntMap.update(id, f, state);
  | BufferUpdate(bu) => IntMap.update(bu.id, applyBufferUpdate(bu), state)
  | BufferSaved(id) => IntMap.update(id, markSaved, state)
  | _ => state
  };
};
