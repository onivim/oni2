[@deriving show]
type t = {search: string => list(string)};

let mergeOptions = options =>
  List.fold_left((accum, opt) => accum ++ " " ++ opt, "", options);

let process = (rgPath, args) => {
  let lines = ref([]);
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
let search = (path, query) =>
  process(path, ["--files", "--sort", "accessed", "--", query]);

let make = path => {search: search(path)};
