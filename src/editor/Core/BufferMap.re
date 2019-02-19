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
type t = Buffers.t(buffer);

let update = (buffersMap, newBuffers) =>
  List.fold_left(
    (m, buffer) => Buffers.add(buffer.id, buffer, m),
    buffersMap,
    newBuffers,
  );

let getActiveBuffer = (activeBufferId, map) =>
  Buffers.find(activeBufferId, map);
