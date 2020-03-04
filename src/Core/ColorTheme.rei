[@deriving show]
type variant =
  | Light
  | Dark
  | HighContrast;

// KEY

type key;
let key: string => key;

// DEFAULTS

module Defaults: {
  type value =
    pri
      | Constant(Revery.Color.t)
      | Reference(string)
      | Computed((string => Revery.Color.t) => Revery.Color.t)
      | Unspecified;

  type entry = {
    light: value,
    dark: value,
    hc: value,
  };

  type t;

  let fromList: list((string, entry)) => t;

  let get: (key, t) => option(entry);

  let union: (t, t) => t;
  let unionMany: list(t) => t;

  module DSL: {
    let hex: string => value;
    let color: Revery.Color.t => value;
    let ref: string => value;
    let computed: ((string => Revery.Color.t) => Revery.Color.t) => value;
    let transparent: (float, value) => value;

    let uniform: value => entry;
  };

  let hex: string => value;
  let color: Revery.Color.t => value;
  let ref: string => value;
  let computed: ((string => Revery.Color.t) => Revery.Color.t) => value;
  let transparent: (float, value) => value;
  let unspecified: value;

  let uniform: value => entry;
};

// COLORS

module Colors: {
  type t;

  let empty: t;
  let fromList: list((string, Revery.Color.t)) => t;

  let get: (key, t) => option(Revery.Color.t);
};

// THEME

type t = {
  variant,
  colors: Colors.t,
};

type resolver = {. color: string => Revery.Color.t};
