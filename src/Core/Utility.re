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
  CamomileBundled.Camomile.(UChar.code(UTF8.get(str, 0)));

let safe_fold_left2 = (fn, accum, list1, list2, ~default) =>
  try(List.fold_left2(fn, accum, list1, list2)) {
  | Invalid_argument(reason) =>
    Log.error("fold_left2 failing because: " ++ reason);
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

// TODO: Remove / replace when upgraded to OCaml 4.08
let filterMap = (f, l) => {
  let rec inner = l =>
    switch (l) {
    | [] => []
    | [hd, ...tail] =>
      switch (f(hd)) {
      | Some(v) => [v, ...inner(tail)]
      | None => inner(tail)
      }
    };

  inner(l);
};

// TODO: Remove / replace with Result.to_option when upgraded to OCaml 4.08
let resultToOption = r => {
  switch (r) {
  | Ok(v) => Some(v)
  | Error(_) => None
  };
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

/**
 * Return the last element in a list.
 */
let rec last =
  fun
  | [] => None
  | [x] => Some(x)
  | [_, ...t] => last(t);

/**
 * Return all but the last element in a list.
 */
let rec dropLast =
  fun
  | [] => []
  | [_] => []
  | [head, ...tail] => [head, ...dropLast(tail)];

let rec firstk = (k, v) =>
  switch (v) {
  | [] => []
  | [hd, ...tail] =>
    if (k <= 1) {
      [hd];
    } else {
      [hd, ...firstk(k - 1, tail)];
    }
  };

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

// TODO: Remove after 4.08 upgrade
module Option = {
  let map = f =>
    fun
    | Some(x) => Some(f(x))
    | None => None;

  let value = (~default) =>
    fun
    | Some(x) => x
    | None => default;

  let iter = f =>
    fun
    | Some(x) => f(x)
    | None => ();

  let iter_none = f =>
    fun
    | Some(_) => ()
    | None => f();

  let some = x => Some(x);

  let bind = f =>
    fun
    | Some(x) =>
      switch (f(x)) {
      | None => None
      | Some(_) as v => v
      }
    | None => None;
};

module Result = {
  let to_option = fun
  | Ok(v) => Some(v)
  | Error(_) => None;
};
