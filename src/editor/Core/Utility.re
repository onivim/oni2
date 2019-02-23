let waitForCondition = (~timeout=1.0, f) => {
  let thread =
    Thread.create(
      () => {
        let s = Unix.gettimeofday();
        while (!f() && Unix.gettimeofday() -. s < timeout) {
          Unix.sleepf(0.0005);
        };
      },
      (),
    );

  Thread.join(thread);
};

let getFileContents = (path, ~handler) => {
  let contents = ref([]);

  let fileInChannel = Pervasives.open_in(path);

  let fileStream =
    Stream.from(_i =>
      switch (Pervasives.input_line(fileInChannel)) {
      | line => Some(line)
      | exception End_of_file => None
      }
    );
  fileStream
  |> Stream.iter(line => {
       let parts = handler(line);
       contents := [parts, ...contents^];
     });

  contents^;
};

let convertUTF8string = str =>
  CamomileLibraryDefault.Camomile.(UChar.code(UTF8.get(str, 0)));

let convertNeovimExtType = (buffer: Msgpck.t) =>
  switch (buffer) {
  | Msgpck.Ext(kind, id) => Some((kind, convertUTF8string(id)))
  | _ => None
  };

let getAtomicCallsResponse = response =>
switch (response) {
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
        | [] => (None, None)
          | [Msgpck.Nil, Msgpck.List(responseItems)] => (
            None,
            Some(responseItems),
          )
          | [errors, Msgpck.List(responseItems)] => (
            Some(errors),
            Some(responseItems),
          )
          | _ => (None, None)
      }
    )
    | _ => (None, None)
};

