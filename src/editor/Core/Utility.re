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

let safe_fold_left2 = (fn, accum, list1, list2, ~default) =>
  try (List.fold_left2(fn, accum, list1, list2)) {
  | Invalid_argument(reason) =>
    print_endline("fold_left2 failing because: " ++ reason);
    default;
  };

let join = paths => {
  let sep = Filename.dir_sep;
  let (head, rest) =
    switch (paths) {
    | [] => ("", [])
    | [head, ...rest] => (head, rest)
    };
  List.fold_left((accum, p) => accum ++ sep ++ p, head, rest);
};

/**
  This is a very rudimentary search case insensitvely checks to see if a substring
  is contained in a larger string.
 */
let contains = (word, substring) => {
  let re = Str.regexp_string_case_fold(substring);
  try (Str.search_forward(re, word, 0) |> ignore |> (_ => true)) {
  | Not_found => false
  };
};
