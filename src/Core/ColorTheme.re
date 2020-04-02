open Revery;

module Log = (val Log.withNamespace("Oni2.Core.ColorTheme"));
module Lookup = Kernel.KeyedStringMap;

// vscode: type
[@deriving show({with_path: false})]
type variant =
  | Light
  | Dark
  | HighContrast;

// KEY

type key = Lookup.key;
let key = Lookup.key;

// COLORS

module Colors = {
  type t = Lookup.t(Color.t);

  let empty = Lookup.empty;
  let fromList = entries => entries |> List.to_seq |> Lookup.of_seq;

  let get = Lookup.find_opt;

  let union = (xs, ys) => Lookup.union((_key, _x, y) => Some(y), xs, ys);
  let unionMany = lookups => List.fold_left(union, Lookup.empty, lookups);
};

// DEFAULTS

module Defaults = {
  // vscode: ColorValue
  type expr =
    | Constant(Color.t)
    | Reference(key)
    | Computed((key => option(Color.t)) => option(Color.t)) // vscode: ColorFunction
    | Unspecified;

  // vscode: ColorDefaults
  type t = {
    light: expr,
    dark: expr,
    hc: expr,
  };

  let get = (variant, defaults) =>
    switch (variant) {
    | Light => defaults.light
    | Dark => defaults.dark
    | HighContrast => defaults.hc
    };

  let evaluate = resolve =>
    fun
    | Constant(color) => Some(color)
    //| Reference(refKey) when refKey == key => failwith("infinite loop detetcted")
    | Reference(refKey) => resolve(refKey)
    | Computed(f) => f(resolve)
    | Unspecified => None;
};

// SCHEMA

module Schema = {
  type definition = {
    key,
    defaults: Defaults.t,
    tryFrom: Colors.t => option(Color.t),
    from: Colors.t => Color.t,
  };

  type t = Lookup.t(definition);

  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(definition => (definition.key, definition))
    |> Lookup.of_seq;

  let get = Lookup.find_opt;

  let union = (xs, ys) =>
    Lookup.union(
      (key, _x, y) => {
        Log.warnf(m =>
          m("Encountered duplicate default: %s", Lookup.keyName(key))
        );
        Some(y);
      },
      xs,
      ys,
    );

  let unionMany = lookups => List.fold_left(union, Lookup.empty, lookups);

  let toList = lookup =>
    Lookup.to_seq(lookup)
    |> Seq.map(((_key, definition)) => definition)
    |> List.of_seq;

  // DSL

  module DSL = {
    open Defaults;

    let hex = str => Constant(Color.hex(str));
    let rgb = (r, g, b) => Constant(Color.rgb_int(r, g, b));
    let rgba = (r, g, b, a) =>
      Constant(Color.rgba_int(r, g, b, int_of_float(a *. 255.)));
    let color = color => Constant(color);
    let ref = def => Reference(def.key);
    let computed = f => Computed(f);
    let unspecified = Unspecified;

    //let darken =
    //let lighten =
    let transparent = (factor, value) => {
      let apply = Color.multiplyAlpha(factor);

      switch (value) {
      | Constant(color) => Constant(color |> apply)
      | Reference(name) =>
        Computed(resolve => resolve(name) |> Option.map(apply))
      | Computed(f) => Computed(resolve => f(resolve) |> Option.map(apply))
      | Unspecified => Unspecified
      };
    };
    let opposite = value => {
      let apply = Color.opposite;

      switch (value) {
      | Constant(color) => Constant(color |> apply)
      | Reference(name) =>
        Computed(resolve => resolve(name) |> Option.map(apply))
      | Computed(f) => Computed(resolve => f(resolve) |> Option.map(apply))
      | Unspecified => Unspecified
      };
    };
    //let oneOf =
    //let lessProminent =

    let all = value => {light: value, dark: value, hc: value};

    let define = (keyName, defaults) => {
      let key = Lookup.key(keyName);

      {
        key,
        defaults,
        tryFrom: Colors.get(key),
        from: lookup =>
          Colors.get(key, lookup)
          |> Option.value(~default=Color.hex("#0000")),
      };
    };
  };

  include DSL;
};

// THEME

type t = {
  variant,
  colors: Colors.t,
};
