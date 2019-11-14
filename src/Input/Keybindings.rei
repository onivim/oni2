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
