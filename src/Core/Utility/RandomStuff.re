open Kernel;

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
  CamomileBundled.Camomile.(UChar.code(UTF8.get(str, 0)));

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
  This is a very rudimentary search, which works case insensitvely
  to see if a substring is contained in a larger string.
 */
let stringContains = (word, substring) => {
  let re = Str.regexp_string_case_fold(substring);
  try(Str.search_forward(re, word, 0) |> ignore |> (_ => true)) {
  | Not_found => false
  };
};

/**
   Get a slice from a list between two indices
 */
let rec sublist = (beginning, terminus, l) =>
  switch (l) {
  | [] => failwith("sublist")
  | [h, ...t] =>
    let tail =
      if (terminus == 0) {
        [];
      } else {
        sublist(beginning - 1, terminus - 1, t);
      };
    if (beginning > 0) {
      tail;
    } else {
      [h, ...tail];
    };
  };

let escapeSpaces = str => {
  let whitespace = Str.regexp(" ");
  Str.global_replace(whitespace, "\\ ", str);
};

// TODO: Remove / replace with Result.to_option when upgraded to OCaml 4.08
let resultToOption = r => {
  switch (r) {
  | Ok(v) => Some(v)
  | Error(_) => None
  };
};

exception ResultError(string);

let resultToException = r => {
  switch (r) {
  | Ok(v) => v
  | Error(msg) => raise(ResultError(msg))
  };
};

let tryToResult = (~msg, f) =>
  try(Ok(f())) {
  | _exn => Error(msg)
  };

type commandLineCompletionMeet = {
  prefix: string,
  position: int,
};

let getCommandLineCompletionsMeet = (str: string, position: int) => {
  let len = String.length(str);

  if (len == 0 || position < len) {
    None;
  } else {
    /* Look backwards for '/' or ' ' */
    let found = ref(false);
    let meet = ref(position);

    while (meet^ > 0 && ! found^) {
      let pos = meet^ - 1;
      let c = str.[pos];
      if (c == ' ') {
        found := true;
      } else {
        decr(meet);
      };
    };

    let pos = meet^;
    Some({prefix: String.sub(str, pos, len - pos), position: pos});
  };
};

let trimTrailingSlash = (item: string) => {
  let len = String.length(item);
  let lastC = item.[len - 1];
  /* Remove trailing slashes */
  if (lastC == '\\' || lastC == '/') {
    String.sub(item, 0, len - 1);
  } else {
    item;
  };
};

let executingDirectory = Revery.Environment.executingDirectory;

external freeConsole: unit => unit = "win32_free_console";

/**
 * Returns `n` bounded by `hi` and `lo`
 *
 * E.g.
 *   clamp(0, ~hi=1, ~lo=-1) == 0
 *   clamp(-1, ~hi=1, ~lo=0) == 0
 *   clamp(1, ~hi=0, ~lo=-1) == 0
 *
 * Assumes `hi` is larger than `lo`
 */
let clamp = (n, ~hi, ~lo) => max(lo, min(hi, n));

/**
 * Returns the list of tuples representing the ranges of consecutive numbers in the input array.
 *
 * E.g.
 *   ranges([|1, 3, 4, 5, 7, 8|]) == [(1, 2), (3, 5), (7, 8)]
 *
 * Assumes the array is sorted in increasing order
 */
let ranges = indices =>
  Array.fold_left(
    (acc, i) =>
      switch (acc) {
      | [] => [(i, i)]

      | [(low, high), ...rest] =>
        if (high + 1 == i) {
          [
            (low, i),
            ...rest // Extend current range
          ];
        } else {
          [
            (i, i),
            ...acc // Add new range
          ];
        }
      },
    [],
    indices,
  )
  |> List.rev;

let tap = (f, x) => {
  f(x);
  x;
};
