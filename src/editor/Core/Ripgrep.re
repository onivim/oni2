let mergeOptions = options =>
  List.fold_left((accum, opt) => accum ++ " " ++ opt, "", options);

let getPath = () =>
  Revery.Environment.(
    switch (os) {
    | Mac
    | Linux =>
      Utility.join([
        getWorkingDirectory(),
        "assets",
        "ripgrep",
        "linux",
        "rg",
      ])
    | Browser
    | Unknown
    | Windows => failwith("not implemented")
    }
  );

let process = args => {
  let lines = ref([]);
  let rgPath = getPath();
  let inChannel = Unix.open_process_in(mergeOptions([rgPath, ...args]));

  Stream.from(_ =>
    switch (input_line(inChannel)) {
    | line => Some(line)
    | exception End_of_file =>
      close_in(inChannel);
      None;
    }
  )
  |> (
    stream =>
      try (Stream.iter(line => lines := [line, ...lines^], stream)) {
      | _error => close_in(inChannel)
      }
  );
  lines^;
};

let search = query => process(["--files", "--", query]);
