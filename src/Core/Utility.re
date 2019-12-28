open EditorCoreTypes;

let identity = v => v;
let noop = () => ();
let noop1 = _ => ();
let noop2 = (_, _) => ();

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

let tap = (f, x) => {
  f(x);
  x;
};

module RangeUtil = {
  let toLineMap: list(Range.t) => IntMap.t(list(Range.t)) =
    ranges => {
      List.fold_left(
        (prev, cur) =>
          Range.(
            IntMap.update(
              Index.toZeroBased(cur.start.line),
              v =>
                switch (v) {
                | None => Some([cur])
                | Some(v) => Some([cur, ...v])
                },
              prev,
            )
          ),
        IntMap.empty,
        ranges,
      );
    };
};

module ArrayEx = {
  exception Found(int);

  let findIndex = (predicate, array) =>
    try(
      {
        for (i in 0 to Array.length(array) - 1) {
          if (predicate(array[i])) {
            raise(Found(i));
          };
        };
        None;
      }
    ) {
    | Found(i) => Some(i)
    };
};

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

  let map2 = (f, a, b) =>
    switch (a, b) {
    | (Some(aVal), Some(bVal)) => Some(f(aVal, bVal))
    | _ => None
    };

  let fallback = f =>
    fun
    | Some(_) as orig => orig
    | None => f();

  let value = (~default) =>
    fun
    | Some(x) => x
    | None => default;

  let iter = f =>
    fun
    | Some(x) => f(x)
    | None => ();

  let iter2 = (f, a, b) => {
    switch (a, b) {
    | (Some(a), Some(b)) => f(a, b)
    | _ => ()
    };
  };

  let iter_none = f =>
    fun
    | Some(_) => ()
    | None => f();

  let tap_none = f =>
    fun
    | Some(_) as v => v
    | None => {
        f();
        None;
      };

  let some = x => Some(x);

  let bind = f =>
    fun
    | Some(x) => f(x)
    | None => None;

  let flatten =
    fun
    | Some(x) => x
    | None => None;

  let join =
    fun
    | Some(x) => x
    | None => None;

  let of_list =
    fun
    | [] => None
    | [hd, ..._] => Some(hd);

  let toString = f =>
    fun
    | Some(v) => Printf.sprintf("Some(%s)", f(v))
    | None => "(None)";

  let values: list(option('a)) => list('a) =
    items => List.filter_map(v => v, items);

  let zip = (a, b) =>
    switch (a, b) {
    | (Some(a), Some(b)) => Some((a, b))
    | _ => None
    };
};

module LwtUtil = {
  let all = (join, promises) => {
    List.fold_left(
      (accPromise, promise) => {
        let%lwt acc = accPromise;
        let%lwt curr = promise;
        Lwt.return(join(acc, curr));
      },
      Lwt.return([]),
      promises,
    );
  };

  exception Timeout;

  let sync: (~timeout: float=?, Lwt.t('a)) => result('a, exn) =
    (~timeout=10.0, promise) => {
      let completed = ref(None);

      Lwt.on_success(promise, v => {completed := Some(Ok(v))});

      Lwt.on_failure(promise, v => {completed := Some(Error(v))});

      waitForCondition(~timeout, () => {completed^ != None});

      Option.value(~default=Error(Timeout), completed^);
    };
};

module Result = {
  let to_option =
    fun
    | Ok(v) => Some(v)
    | Error(_) => None;

  let bind = f => fun
  | Ok(v) => f(v)
  | Error(_) as err => err;

  let map = f => fun
  | Ok(v) => Ok(f(v))
  | Error(_) as err => err;

  let default = (~value) => fun
  | Ok(v) => v
  | Error(_) => value;

  let exn = fun
  | Ok(v) => v
  | Error(msg) => raise(ResultError(msg));
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

  /** [contains(query, str)] returns true if [str] contains the substring [query], false otherwise. */
  let contains = (query, str) => {
    let re = Str.regexp_string(query);
    try({
      let _: int = Str.search_forward(re, str, 0);
      true;
    }) {
    | Not_found => false
    };
  };

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

module type Queue = {
  type t('a);

  let empty: t(_);
  let length: t('a) => int;
  let isEmpty: t('a) => bool;
  let push: ('a, t('a)) => t('a);
  let pushFront: ('a, t('a)) => t('a);
  let pop: t('a) => (option('a), t('a));
  let take: (int, t('a)) => (list('a), t('a));
  let toList: t('a) => list('a);
};

module Queue: Queue = {
  type t('a) = {
    front: list('a),
    rear: list('a), // reversed
    length: int,
  };

  let empty = {front: [], rear: [], length: 0};

  let length = queue => queue.length;
  let isEmpty = queue => queue.length == 0;

  let push = (item, queue) => {
    ...queue,
    rear: [item, ...queue.rear],
    length: queue.length + 1,
  };

  let pushFront = (item, queue) => {
    ...queue,
    front: [item, ...queue.front],
    length: queue.length + 1,
  };

  let pop = queue => {
    let queue =
      if (queue.front == []) {
        {...queue, front: List.rev(queue.rear), rear: []};
      } else {
        queue;
      };

    switch (queue.front) {
    | [item, ...tail] => (
        Some(item),
        {...queue, front: tail, length: queue.length - 1},
      )
    | [] => (None, queue)
    };
  };

  let rec take = (count, queue) =>
    if (count == 0) {
      ([], queue);
    } else {
      switch (pop(queue)) {
      | (Some(item), queue) =>
        let (items, queue) = take(count - 1, queue);
        ([item, ...items], queue);

      | (None, queue) => ([], queue)
      };
    };

  let toList = ({front, rear, _}) => front @ List.rev(rear);
};

module ChunkyQueue: {
  include Queue;

  let pushChunk: (list('a), t('a)) => t('a);
  let pushReversedChunk: (list('a), t('a)) => t('a);
} = {
  type t('a) = {
    front: list('a),
    rear: Queue.t(list('a)),
    length: int,
  };

  let empty = {front: [], rear: Queue.empty, length: 0};

  let length = queue => queue.length;
  let isEmpty = queue => queue.length == 0;

  let push = (item, queue) => {
    ...queue,
    rear: Queue.push([item], queue.rear),
    length: queue.length + 1,
  };

  let pushReversedChunk = (chunk, queue) =>
    if (chunk == []) {
      queue;
    } else {
      {
        ...queue,
        rear: Queue.push(chunk, queue.rear),
        length: queue.length + List.length(chunk),
      };
    };

  let pushChunk = chunk => pushReversedChunk(List.rev(chunk));

  let pushFront = (item, queue) => {
    ...queue,
    front: [item, ...queue.front],
    length: queue.length + 1,
  };

  let pop = queue => {
    let queue =
      if (queue.front == []) {
        switch (Queue.pop(queue.rear)) {
        | (Some(chunk), rear) => {...queue, front: chunk, rear}
        | (None, rear) => {...queue, front: [], rear}
        };
      } else {
        queue;
      };

    switch (queue.front) {
    | [item, ...tail] => (
        Some(item),
        {...queue, front: tail, length: queue.length - 1},
      )
    | [] => (None, queue)
    };
  };

  let rec take = (count, queue) =>
    if (count == 0) {
      ([], queue);
    } else {
      switch (pop(queue)) {
      | (Some(item), queue) =>
        let (items, queue) = take(count - 1, queue);
        ([item, ...items], queue);

      | (None, queue) => ([], queue)
      };
    };

  let toList = ({front, rear, _}) =>
    front @ (Queue.toList(rear) |> List.concat);
};

module Path = {
  // Not very robust path-handling utilities.
  // TODO: Make good

  let toRelative = (~base, path) => {
    let base = base == "/" ? base : base ++ Filename.dir_sep;
    Str.replace_first(Str.regexp_string(base), "", path);
  };

  let explode = String.split_on_char(Filename.dir_sep.[0]);
};

module Json = {
  /*
    update method adapted from: https://github.com/ocaml-community/yojson/issues/54
   */
  let update = (key, f, json) => {
    let rec update_json_obj =
      fun
      | [] =>
        switch (f(None)) {
        | None => []
        | Some(v) => [(key, v)]
        }
      | [(k, v) as m, ...tail] as original =>
        if (String.equal(k, key)) {
          switch (f(Some(v))) {
          | None => update_json_obj(tail)
          | Some(v') =>
            if (v' == v) {
              original;
            } else {
              [(k, v'), ...tail];
            }
          };
        } else {
          [m, ...update_json_obj(tail)];
        };

    switch (json) {
    | `Assoc(items) => `Assoc(update_json_obj(items))
    | _ => json
    };
  };

  let getKeys = json => {
    let rec loop = (curr, json) => {
      switch (json) {
      | `Assoc(items) =>
        items
        |> List.map(item => {
             let (key, v) = item;
             let prefix = curr == "" ? key : curr ++ "." ++ key;
             loop(prefix, v);
           })
        |> List.flatten
      | _ => [curr]
      };
    };

    loop("", json);
  };

  let explode_key = String.split_on_char('.');
  /*
   [explode(json)] takes a JSON structure like:
   [{ "a.b.c": 1}]

   and converts it to:
   [{"a": { "b": { "c": 1 }}}]
   */

  let explode = json => {
    let rec expand_item = (currJson, keys, jsonValue) => {
      switch (keys) {
      | [key] => update(key, _ => Some(jsonValue), currJson)
      | [key, ...remaining] =>
        update(
          key,
          fun
          | None => Some(expand_item(`Assoc([]), remaining, jsonValue))
          | Some(json) => Some(expand_item(json, remaining, jsonValue)),
          currJson,
        )

      | [] => jsonValue // Shouldn't hit this case...
      };
    }
    and expand_items = items => {
      List.fold_left(
        (acc, curr) => {
          let (key, jsonValue) = curr;

          let explodedKey = explode_key(key);
          expand_item(acc, explodedKey, loop(jsonValue));
        },
        `Assoc([]),
        items,
      );
    }
    and loop = json => {
      switch (json) {
      | `Assoc(items) => expand_items(items)
      | v => v
      };
    };

    loop(json);
  };
};
