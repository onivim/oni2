module Keybinding: {
  type t = {
    key: string,
    command: string,
    condition: Expression.t,
  };
};

type t = list(Keybinding.t);

let default: t;

/*
  [get] reads the keybindings from the file system
 */
let get: unit => t;

/*
   [of_yojson_with_errors] parses the keybindings,
   and returns the list of successfully parsed keybindings,
   as well as a list of errors for unsuccessfully parsed keybindings.
 */
let of_yojson_with_errors:
  Yojson.Safe.t => result((t, list(string)), string);
