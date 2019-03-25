let mergeOptions = options =>
  List.fold_left((accum, opt) => accum ++ " " ++ opt, "", options);

let getPath = () =>
  Utility.(
    Revery.Environment.(
      join([getWorkingDirectory(), "assets", "ripgrep"])
      |> (
        grepDir =>
          switch (os) {
          | Mac => join([grepDir, "mac", "rg"])
          | Linux => join([grepDir, "linux", "rg"])
          | Windows => join([grepDir, "windows", "rg.exe"])
          | Browser
          | Unknown => failwith("not implemented")
          }
      )
    )
  );

let process = args => {
  let lines = ref([]);
  let rgPath = getPath();
  /**
     NOTE: this is run in parallel to the main program
   */
  let inChannel = mergeOptions([rgPath, ...args]) |> Unix.open_process_in;

  Stream.from(_ =>
    switch (input_line(inChannel)) {
    | line => Some(line)
    | exception End_of_file => None
    }
  )
  |> (
    stream =>
      try (Stream.iter(line => lines := [line, ...lines^], stream)) {
      | _error => close_in(inChannel)
      }
  );
  close_in(inChannel);
  lines^;
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = query => process(["--files", "--sort", "accessed", "--", query]);
