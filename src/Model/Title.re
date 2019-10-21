/*
 * Title.re
 *
 * Model for working with the window title
 */

open Oni_Core;

type titleSections =
  | Text(string)
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
      result := [Text(String.sub(str, idx^, len - idx^)), ...result^];
      idx := len;
    | v =>
      let prev = v - idx^;

      if (prev > 0) {
        result := [Text(String.sub(str, idx^, prev)), ...result^];
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
    | Text(sz) => Some(Text(sz))
    | Separator => Some(Separator)
    | Variable(sz) =>
      switch (StringMap.find_opt(sz, items)) {
      | Some(v) => Some(Text(v))
      | None => None
      }
    };
  };

  v |> List.map(f) |> Utility.filterMap(v => v);
};

let toString = (v: t, items: StringMap.t(string)) => {
  let resolvedItems = _resolve(v, items);

  let rec f = (v: t, hadText) => {
    switch (v) {
    | [Separator, Text(t2), ...tail] when hadText =>
      " - " ++ t2 ++ f(tail, true)
    | [Text(t), ...tail] => t ++ f(tail, true)
    | [Separator, ...tail] => f(tail, false)
    | [Variable(_v), ...tail] => f(tail, false)
    | [] => ""
    };
  };

  f(resolvedItems, false);
};
