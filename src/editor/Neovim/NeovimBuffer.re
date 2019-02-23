open Oni_Core;
open Types;

let constructMetadataCalls = id => [
  Msgpck.List([
    Msgpck.String("nvim_call_function"),
    Msgpck.List([
      Msgpck.String("OniGetBufferContext"),
      Msgpck.List([Msgpck.Int(id)]),
    ]),
  ]),
];

let parseBufferContext = map =>
  List.fold_left(
    (accum: BufferMetadata.t, item) =>
      switch (item) {
      | (Msgpck.String("bufferFullPath"), Msgpck.String(value)) => {
          ...accum,
          filePath: Some(value),
        }
      | (Msgpck.String("bufferNumber"), Msgpck.Int(bufNum)) => {
          ...accum,
          id: bufNum,
        }
      | (Msgpck.String("modified"), Msgpck.Int(modified)) => {
          ...accum,
          modified: modified == 1,
        }
      | (Msgpck.String("buftype"), Msgpck.String(buftype)) => {
          ...accum,
          bufType: getBufType(buftype),
        }
      | (Msgpck.String("filetype"), Msgpck.String(fileType)) => {
          ...accum,
          fileType: Some(fileType),
        }
      | (Msgpck.String("hidden"), Msgpck.Bool(hidden)) => {
          ...accum,
          hidden,
        }
      | _ => accum
      },
    BufferMetadata.create(),
    map,
  );

/**
    Enrich Buffers

    Neovim allows us to get a list of all buffer ids
    we then use the IDs of the buffers to request more
    metadata about each buffer using "atomic calls"
   */
let enrichBuffers = (api: NeovimApi.t, (atomicCalls, buffers)) => {
  let rsp =
    api.requestSync(
      "nvim_call_atomic",
      Msgpck.List([Msgpck.List(atomicCalls)]),
    )
    |> Utility.getAtomicCallsResponse;

  switch (rsp) {
  | (_, Some(items)) =>
    List.fold_left2(
      (accum, buf, bufferContext) =>
        switch (bufferContext) {
        | Msgpck.Map(context) => [parseBufferContext(context), ...accum]
        | _ => [buf, ...accum]
        },
      [],
      buffers,
      items,
    )
  | _ => []
  };
};

let filterInvalidBuffers = (api: NeovimApi.t, buffers) => {
  let calls =
    List.map(
      ((_, id)) =>
        Msgpck.List([
          Msgpck.String("nvim_buf_is_loaded"),
          Msgpck.List([Msgpck.Int(id)]),
        ]),
      buffers,
    );

  let (_errors, listOfBooleans) =
    api.requestSync("nvim_call_atomic", Msgpck.List([Msgpck.List(calls)]))
    |> Utility.getAtomicCallsResponse;

  switch (listOfBooleans) {
  | Some(booleans) =>
    List.fold_left2(
      (accum, buf, isValid) =>
        switch (isValid) {
        | Msgpck.Bool(true) => [buf, ...accum]
        | Msgpck.Bool(false)
        | _ => accum
        },
      [],
      buffers,
      booleans,
    )
  | None => []
  };
};

let unwrapBufferList = msgs =>
  switch (msgs) {
  | Msgpck.List(handles) =>
    List.fold_left(
      (accum, buf) =>
        switch (Utility.convertNeovimExtType(buf)) {
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
  api.requestSync("nvim_list_bufs", Msgpck.List([]))
  |> unwrapBufferList
  |> filterInvalidBuffers(api)
  |> List.fold_left(
       ((calls, bufs), (_, id)) => {
         let newCalls =
           constructMetadataCalls(id) |> List.append(calls) |> List.rev;

         let newBuffers =
           [
             BufferMetadata.create(
               ~id,
               ~modified=false,
               ~hidden=false,
               ~bufType=Empty,
               ~fileType=None,
               ~filePath=Some("[No Name]"),
               (),
             ),
             ...bufs,
           ]
           |> List.rev;

         (newCalls, newBuffers);
       },
       ([], []),
     )
  |> enrichBuffers(api);
