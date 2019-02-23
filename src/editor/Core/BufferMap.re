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

type t = Buffers.t(Buffer.t);

let empty = Buffers.empty;

type mapFunction = Buffer.t => Buffer.t;

let map = Buffers.map;
let update = Buffers.update;
let remove = Buffers.remove;

let updateMetadata = (buffersMap: t, newBuffers: list(BufferMetadata.t)) => {
  /**
     The update function does not remove old buffers from the BufferMap so
     this function filters the map for any buffers that aren't in the new update
     and removes them. As the new buffers are an up-to-date list of existing buffers

     NOTE: an alternative could be NOT filtering out unloaded buffers in NeovimBuffer
     but setting the unlisted key and checking it in the update below and removing
     buffers in the update based on that key....
   */
  let filteredMap =
    Buffers.filter(
      (_, b: Buffer.t) =>
        List.exists(
          (newBuf: BufferMetadata.t) => newBuf.id == b.metadata.id,
          newBuffers,
        ),
      buffersMap,
    );
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
    filteredMap,
    newBuffers,
  );
};

let log = (m: t) =>
  Buffers.iter(
    (_, b) =>
      Buffer.show(b)
      |> (++)("Buffer =======================: \n")
      |> print_endline,
    m,
  );

let getBuffer = (id, map) => Buffers.find_opt(id, map);
