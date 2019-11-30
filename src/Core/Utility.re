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
module List = {
  include List;

  let filter_map = f => {
    let rec aux = accu =>
      fun
      | [] => List.rev(accu)
      | [x, ...l] =>
        switch (f(x)) {
        | None => aux(accu, l)
        | Some(v) => aux([v, ...accu], l)
        };

    aux([]);
  };
};

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

  let map2 = (f, a, b) => 
    switch (a, b) {
    | (Some(aVal), Some(bVal)) => Some(f(aVal, bVal))
    | _ => None;
    }

  let bind = f =>
    fun
    | Some(x) => f(x)
    | None => None;

  let join =
    fun
    | Some(x) => x
    | None => None;
};

module Result = {
  let to_option =
    fun
    | Ok(v) => Some(v)
    | Error(_) => None;
};

module StringUtil = {
  let isSpace =
    fun
    | ' '
    | '\012'
    | '\n'
    | '\r'
    | '\t' => true
    | _ => false;

  let trimLeft = str => {
    let length = String.length(str);

    let rec aux = i =>
      if (i >= length) {
        "";
      } else if (isSpace(str.[i])) {
        aux(i + 1);
      } else if (i == 0) {
        str;
      } else {
        String.sub(str, i, length - i);
      };

    aux(0);
  };

  let trimRight = str => {
    let length = String.length(str);

    let rec aux = j =>
      if (j <= 0) {
        "";
      } else if (isSpace(str.[j])) {
        aux(j - 1);
      } else if (j == length - 1) {
        str;
      } else {
        String.sub(str, 0, j + 1);
      };

    aux(length - 1);
  };

  let extractSnippet = (~maxLength, ~charStart, ~charEnd, text) => {
    let originalLength = String.length(text);

    let indentation = {
      let rec aux = i =>
        if (i >= originalLength) {
          originalLength;
        } else if (isSpace(text.[i])) {
          aux(i + 1);
        } else {
          i;
        };

      aux(0);
    };

    let remainingLength = originalLength - indentation;
    let matchLength = charEnd - charStart;

    if (remainingLength > maxLength) {
      // too long

      let (offset, length) =
        if (matchLength >= maxLength) {
          (
            // match is lonegr than allowed
            charStart,
            maxLength,
          );
        } else if (charEnd - indentation > maxLength) {
          (
            // match ends out of bounds
            charEnd - maxLength,
            maxLength,
          );
        } else {
          (
            // match is within bounds
            indentation,
            maxLength,
          );
        };

      if (offset > indentation) {
        // We're cutting non-indentation from the start, so add ellipsis

        let ellipsis = "...";
        let ellipsisLength = String.length(ellipsis);

        // adjust for ellipsis
        let (offset, length) = {
          let availableEnd = length - (charEnd - offset);

          if (ellipsisLength > length) {
            (
              // ellipsis won't even fit... not much to do then I guess
              offset,
              length,
            );
          } else if (ellipsisLength <= availableEnd) {
            (
              // fits at the end, so take it from there
              offset,
              length - ellipsisLength,
            );
          } else {
            // won't fit at the end
            let remainder = ellipsisLength - availableEnd;

            if (remainder < charStart - offset) {
              (
                // remainder will fit at start
                offset + remainder,
                length - ellipsisLength,
              );
            } else {
              (
                // won't fit anywhere, so just chop it off the end
                offset,
                length - ellipsisLength,
              );
            };
          };
        };

        (
          ellipsis ++ String.sub(text, offset, length),
          ellipsisLength + charStart - offset,
          ellipsisLength + min(length, charEnd - offset),
        );
      } else {
        (
          // We're only cutting indentation from the start
          String.sub(text, offset, length),
          charStart - offset,
          min(length, charEnd - offset),
        );
      };
    } else if (indentation > 0) {
      // not too long, but there's indentation

      // do not remove indentation included in match
      let offset = indentation > charStart ? charStart : indentation;
      let length = min(maxLength, originalLength - offset);

      (
        String.sub(text, offset, length),
        charStart - offset,
        charEnd - offset,
      );
    } else {
      (
        // not too long, no indentation
        text,
        charStart,
        charEnd,
      );
    };
  };
};
