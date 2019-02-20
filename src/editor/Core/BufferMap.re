open Types;
/*
   This creates a Map with uses an integer
   as the key, this is not offered by default.
   but you can pass a Functor
   with a type that satisfies the interface
   i.e. specifies a compare function and a type
 */
module Buffers =
  Map.Make({
    type t = int;
    let compare = compare;
  });

let empty = Buffers.empty;
type t = Buffers.t(Buffer.t);

type mapFunction = Buffer.t => Buffer.t;

let map = Buffers.map;
let update = Buffers.update;

let updateMetadata = (buffersMap: t, newBuffers: list(BufferMetadata.t)) =>
  List.fold_left(
    (m: t, metadata: BufferMetadata.t) =>
      Buffers.update(
        metadata.id,
        original =>
          switch (original) {
          | None => Some(Buffer.ofMetadata(metadata))
          | Some(v) => Some({...v, metadata}: Buffer.t)
          },
        m,
      ),
    buffersMap,
    newBuffers,
  );

let getBuffer = (id, map) => Buffers.find(id, map);
