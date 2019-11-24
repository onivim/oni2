/*
 * Title.re
 *
 * Model for working with the window title
 */

open Oni_Core;

module List = Utility.List;

type titleSections =
  | Text(string, bool)
  | Separator
  | Variable(string);

type t = list(titleSections);

let regexp = Str.regexp("\\${\\([a-zA-Z0-9]+\\)}");

let ofString = str => {
  let idx = ref(0);
  let len = String.length(str);
  let result = ref([]);
  while (idx^ < len) {
    switch (Str.search_forward(regexp, str, idx^)) {
    | exception Not_found =>
      result := [Text(String.sub(str, idx^, len - idx^), false), ...result^];
      idx := len;
    | v =>
      let prev = v - idx^;

      if (prev > 0) {
        result := [Text(String.sub(str, idx^, prev), false), ...result^];
      };

      let group = Str.matched_group(1, str);
      idx := v + String.length(group) + 3;
      if (String.equal(group, "separator")) {
        result := [Separator, ...result^];
      } else {
        result := [Variable(group), ...result^];
      };
    };
  };

  result^ |> List.rev;
};

let _resolve = (v: t, items: StringMap.t(string)) => {
  let f = section => {
    switch (section) {
    | Text(sz, fromVariable) => Some(Text(sz, fromVariable))
    | Separator => Some(Separator)
    | Variable(sz) =>
      switch (StringMap.find_opt(sz, items)) {
      | Some(v) => Some(Text(v, true))
      | None => None
      }
    };
  };

  v |> List.map(f) |> List.filter_map(v => v);
};

let toString = (v: t, items: StringMap.t(string)) => {
  let resolvedItems = _resolve(v, items);

  let rec f = (v: t, hadText) => {
    switch (v) {
    | [Separator, Text(t2, true), ...tail] when hadText =>
      " - " ++ t2 ++ f(tail, true)
    | [Text(t, true), ...tail] => t ++ f(tail, true)
    | [Text(t, false), ...tail] => t ++ f(tail, false)
    | [Separator, ...tail] => f(tail, hadText)
    | [Variable(_v), ...tail] => f(tail, false)
    | [] => ""
    };
  };

  f(resolvedItems, false);
};
