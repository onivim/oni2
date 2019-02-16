open Types;
/*
   This creates a Map with uses an integer
   as the key, you can pass a Functor/Module
   with a type that satisfies the interface
   i.e. specifies a compare function and a type
 */
module OniBuffers =
  Map.Make({
    type t = int;
    let compare = compare;
  });

let empty = OniBuffers.empty;
type map = OniBuffers.t(StateBuffer.t);

let update = (buffersMap, newBuffers) => {
  List.fold_left(
    (m, buffer) => OniBuffers.add(StateBuffer.(buffer.id), buffer, m),
    buffersMap,
    newBuffers,
  );
};
