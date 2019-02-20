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
    );
  switch (rsp) {
  | Msgpck.List(result) =>
    /**
        The last argument in an atomic call is either NIL or an
        array of three items  a three-element array with the zero-based
        index of the call which resulted in an error, the error type
        and the error message. If an error occurred,
        the values from all preceding calls will still be returned.
       */
    (
      switch (List.rev(result)) {
      | [] => []
      | [_errors, ...responseItems] =>
        switch (responseItems) {
        | [Msgpck.List(items)] =>
          List.fold_left2(
            (accum, buf, bufferContext) =>
              switch (bufferContext) {
              | Msgpck.Map(context) => [
                  parseBufferContext(context),
                  ...accum,
                ]
              | _ => [buf, ...accum]
              },
            [],
            buffers,
            items,
          )
        | _ => []
        }
      }
    )
  | _ => []
  };
};

let getBufferList = (api: NeovimApi.t) => {
  let bufs = api.requestSync("nvim_list_bufs", Msgpck.List([]));
  (
    switch (bufs) {
    | Msgpck.List(handles) =>
      List.fold_left(
        ((calls, bufs), buffer) =>
          switch (Utility.convertNeovimExtType(buffer)) {
          | Some((_, id)) =>
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
          | None => (calls, bufs)
          },
        ([], []),
        handles,
      )
    | _ => ([], [])
    }
  )
  |> enrichBuffers(api);
};
