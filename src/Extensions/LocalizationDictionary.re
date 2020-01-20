/*
 * LocalizationDictionary.re
 */

open Oni_Core;
open Oni_Core_Kernel;
open Oni_Core_Utility;

type t = StringMap.t(string);

let initial = StringMap.empty;

let to_string_map = items => {
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
  | `Assoc(items) => to_string_map(items)
  | _ => StringMap.empty
  };
};

let get = StringMap.find_opt;

let count = v => StringMap.bindings(v) |> List.length;
