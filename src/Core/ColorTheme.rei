[@deriving show]
type variant =
  | Light
  | Dark
  | HighContrast;

// KEY

type key;
let key: string => key;

// COLORS

module Colors: {
  type t;

  let empty: t;
  let fromList: list((key, Revery.Color.t)) => t;

  let get: (key, t) => option(Revery.Color.t);

  let union: (t, t) => t;
  let unionMany: list(t) => t;
};

// DEFAULTS

module Defaults: {
  type expr;

  type t = {
    light: expr,
    dark: expr,
    hc: expr,
  };

  let get: (variant, t) => expr;
  let evaluate:
    (key => option(Revery.Color.t), expr) => option(Revery.Color.t);
};

// SCHEMA

module Schema: {
  type definition = {
    key,
    defaults: Defaults.t,
    tryFrom: Colors.t => option(Revery.Color.t),
    from: Colors.t => Revery.Color.t,
  };

  type t;

  let fromList: list(definition) => t;

  let get: (key, t) => option(definition);

  let union: (t, t) => t;
  let unionMany: list(t) => t;

  let toList: t => list(definition);

  module DSL: {
    let hex: string => Defaults.expr;
    let rgb: (int, int, int) => Defaults.expr;
    let rgba: (int, int, int, float) => Defaults.expr;
    let color: Revery.Color.t => Defaults.expr;
    let ref: definition => Defaults.expr;
    let computed:
      ((key => option(Revery.Color.t)) => option(Revery.Color.t)) =>
      Defaults.expr;
    let unspecified: Defaults.expr;

    let transparent: (float, Defaults.expr) => Defaults.expr;
    let opposite: Defaults.expr => Defaults.expr;

    let all: Defaults.expr => Defaults.t;

    let define: (string, Defaults.t) => definition;
  };

  let hex: string => Defaults.expr;
  let rgb: (int, int, int) => Defaults.expr;
  let rgba: (int, int, int, float) => Defaults.expr;
  let color: Revery.Color.t => Defaults.expr;
  let ref: definition => Defaults.expr;
  let computed:
    ((key => option(Revery.Color.t)) => option(Revery.Color.t)) =>
    Defaults.expr;
  let unspecified: Defaults.expr;

  let transparent: (float, Defaults.expr) => Defaults.expr;
  let opposite: Defaults.expr => Defaults.expr;

  let all: Defaults.expr => Defaults.t;

  let define: (string, Defaults.t) => definition;
};

// THEME

type t = {
  variant,
  colors: Colors.t,
};
