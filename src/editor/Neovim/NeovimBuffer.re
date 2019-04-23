open Oni_Core;
open Types;
open NeovimHelpers;

module M = Msgpck;

let parseBufferContext = map => {
  List.fold_left(
    (accum: BufferMetadata.t, item) =>
      switch (item) {
      | (M.String("bufferFullPath"), M.String(value)) => {
          ...accum,
          filePath: value == "" ? None : Some(value),
        }
      | (M.String("bufferNumber"), M.Int(bufNum)) => {...accum, id: bufNum}
      /* TODO */
      | (M.String("modified"), M.Int(modified)) => {
          ...accum,
          modified: modified === 1,
        }
      | (M.String("buftype"), M.String(buftype)) => {
          ...accum,
          bufType: getBufType(buftype),
        }
      | (M.String("changedtick"), M.Int(version)) => {...accum, version}
      | (M.String("hidden"), M.Bool(hidden)) => {...accum, hidden}
      | _ => accum
      },
    BufferMetadata.create(),
    map,
  );
};

let getContext = (api: NeovimApi.t, id) => {
  let v =
    api.requestSync(
      "nvim_call_function",
      M.List([M.String("OniGetBufferContext"), M.List([M.Int(id)])]),
    );
  switch (v) {
  | M.Map(context) => parseBufferContext(context)
  /* TODO: Assert */
  | _ => BufferMetadata.create()
  };
};
