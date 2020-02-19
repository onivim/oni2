module Keybinding: {
  type t = {
    key: string,
    command: string,
    condition: WhenExpression.t,
  };
};

type t = list(Keybinding.t);

let empty: t;

/*
   [of_yojson_with_errors] parses the keybindings,
   and returns the list of successfully parsed keybindings,
   as well as a list of errors for unsuccessfully parsed keybindings.
 */
let of_yojson_with_errors:
  Yojson.Safe.t => result((t, list(string)), string);
