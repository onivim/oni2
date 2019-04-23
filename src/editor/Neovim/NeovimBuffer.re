open Oni_Core;
open Types;
open NeovimHelpers;

module M = Msgpck;

type partialBuffer = {id: int};


let constructMetadataCalls = ((_, id)) => [
  M.List([
    M.String("nvim_call_function"),
    M.List([M.String("OniGetBufferContext"), M.List([M.Int(id)])]),
  ]),
];

let parseBufferContext = map =>
  List.fold_left(
    (accum: BufferMetadata.t, item) =>
      switch (item) {
      | (M.String("bufferFullPath"), M.String(value)) => {
          ...accum,
          filePath: value == "" ? None : Some(value),
        }
      | (M.String("bufferNumber"), M.Int(bufNum)) => {...accum, id: bufNum}
      /* TODO */
      | (M.String("modified"), M.Int(modified)) => accum
      | (M.String("buftype"), M.String(buftype)) => {
          ...accum,
          bufType: getBufType(buftype),
        }
      | (M.String("hidden"), M.Bool(hidden)) => {...accum, hidden}
      | _ => accum
      },
    BufferMetadata.create(),
    map,
  );

let getContext = (api: NeovimApi.t, id) => {
   let v = api.requestSync("nvim_call_function", M.List([M.String("OniGetBufferContext"), M.List([M.Int(id)])]))
    switch (v) {
    | M.Map(context) => parseBufferContext(context)
    /* TODO: Assert */ 
    | _ => BufferMetadata.create();
    }
};

/**
    Enrich Buffers

    Neovim allows us to get a list of all buffer ids
    we then use the IDs of the buffers to request more
    metadata about each buffer using "atomic calls"
   */
let enrichBuffers = (api: NeovimApi.t, atomicCalls) => {
  let rsp =
    api.requestSync("nvim_call_atomic", M.List([M.List(atomicCalls)]))
    |> getAtomicCallsResponse;

  switch (rsp) {
  | (_, Some(items)) =>
    List.fold_left(
      (accum, bufferContext) =>
        switch (bufferContext) {
        | M.Map(context) => [parseBufferContext(context), ...accum]
        | _ => accum
        },
      [],
      items,
    )
  | _ => []
  };
};

let filterInvalidBuffers = (api: NeovimApi.t, buffers) => {
  let calls =
    List.map(
      ((_, id)) =>
        M.List([M.String("nvim_buf_is_loaded"), M.List([M.Int(id)])]),
      buffers,
    );

  let (_errors, listOfBooleans) =
    api.requestSync("nvim_call_atomic", M.List([M.List(calls)]))
    |> getAtomicCallsResponse;

  switch (listOfBooleans) {
  | Some(booleans) =>
    Utility.safe_fold_left2(
      (accum, buf, isValid) =>
        switch (isValid) {
        | M.Bool(true) => [buf, ...accum]
        | M.Bool(false)
        | _ => accum
        },
      [],
      buffers,
      booleans,
      ~default=buffers,
    )
  | None => []
  };
};

let unwrapBufferList = msgs =>
  switch (msgs) {
  | M.List(handles) =>
    List.fold_left(
      (accum, buf) =>
        switch (convertNeovimExtType(buf)) {
        | Some(bf) => [bf, ...accum]
        | None => accum
        },
      [],
      handles,
    )
    |> List.rev
  | _ => []
  };

let getBufferList = (api: NeovimApi.t) =>
  api.requestSync("nvim_list_bufs", M.List([]))
  |> unwrapBufferList
  |> filterInvalidBuffers(api)
  |> List.map(constructMetadataCalls)
  |> List.flatten
  |> enrichBuffers(api);
