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
