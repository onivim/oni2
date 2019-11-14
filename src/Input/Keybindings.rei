open Types.Input;

module Keybinding = {
  type t = {
    key: string,
    command: string,
    condition: Expression.t,
  };
};

type t = list(Keybinding.t);

let default: t;

let getDefaultConfig: unit => t;
