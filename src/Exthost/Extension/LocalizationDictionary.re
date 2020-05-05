/*
 * LocalizationDictionary.re
 */

open Oni_Core;

type t = StringMap.t(string);

let initial = StringMap.empty;

let toStringMap = items => {
  List.fold_left(
    (acc, curr) => {
      let (key, json) = curr;
      switch (json) {
      | `String(v) => StringMap.add(key, v, acc)
      | _ => acc
      };
    },
    StringMap.empty,
    items,
  );
};

let of_yojson = json => {
  switch (json) {
  | `Assoc(items) => toStringMap(items)
  | _ => StringMap.empty
  };
};

let get = StringMap.find_opt;

let count = v => StringMap.bindings(v) |> List.length;
