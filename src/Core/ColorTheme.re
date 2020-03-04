open Revery;

module Log = (val Log.withNamespace("Oni2.Core.ColorTheme"));

// vscode: type
[@deriving show({with_path: false})]
type variant =
  | Light
  | Dark
  | HighContrast;

// INTERNAL

module Internal = {
  module Key: {
    type t =
      pri {
        hash: int,
        name: string,
      };
    let compare: (t, t) => int;
    let create: string => t;
  } = {
    type t = {
      hash: int,
      name: string,
    };

    let compare = (a, b) =>
      a.hash == b.hash ? compare(a.name, b.name) : compare(a.hash, b.hash);

    let create = name => {hash: Hashtbl.hash(name), name};
  };

  module Lookup = Map.Make(Key);
};

// KEY

type key = Internal.Key.t;
let key = Internal.Key.create;

// DEFAULTS

module Defaults = {
  type value =
    | Constant(Color.t)
    | Reference(string)
    | Computed((string => Color.t) => Color.t) // vscode: ColorFunction
    | Unspecified;

  // vscode: ColorDefaults
  type entry = {
    light: value,
    dark: value,
    hc: value,
  };

  type t = Internal.Lookup.t(entry);

  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(((keyName, entry)) =>
         (Internal.Key.create(keyName), entry)
       )
    |> Internal.Lookup.of_seq;

  let get = Internal.Lookup.find_opt;

  let union = (xs, ys) =>
    Internal.Lookup.union(
      (key, _x, y) => {
        Log.warnf(m => m("Encountered duplicate default: %s", key.name));
        Some(y);
      },
      xs,
      ys,
    );

  let unionMany = lookups =>
    List.fold_left(union, Internal.Lookup.empty, lookups);

  // DSL

  module DSL = {
    let hex = str => Constant(Color.hex(str));
    let color = color => Constant(color);
    let ref = str => Reference(str);
    let computed = f => Computed(f);
    //let darken =
    //let lighten =
    let transparent = (factor, value) => {
      let apply = Color.multiplyAlpha(factor);

      switch (value) {
      | Constant(color) => Constant(color |> apply)
      | Reference(name) => Computed(resolve => resolve(name) |> apply)
      | Computed(f) => Computed(resolve => f(resolve) |> apply)
      | Unspecified => Constant(Colors.black |> apply)
      };
    };
    //let oneOf =
    //let lessProminent =
    let unspecified = Unspecified;

    let uniform = value => {light: value, dark: value, hc: value};
  };

  include DSL;
};

// COLORS

module Colors = {
  type t = Internal.Lookup.t(Color.t);

  let empty = Internal.Lookup.empty;
  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(((keyName, entry)) =>
         (Internal.Key.create(keyName), entry)
       )
    |> Internal.Lookup.of_seq;

  let get = Internal.Lookup.find_opt;
};

// THEME

type t = {
  variant,
  colors: Colors.t,
};

type resolver = {. color: string => Color.t};
