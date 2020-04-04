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

// RESOLVER

type resolver =
  key => [ | `Color(Color.t) | `Default(Defaults.expr) | `NotRegistered];

// SCHEMA

module Schema = {
  type definition = {
    key,
    defaults: Defaults.t,
    tryFrom: resolver => option(Color.t),
    from: resolver => Color.t,
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

      let rec tryGet = (resolve, key) =>
        switch (resolve(key)) {
        | `Color(color) => Some(color)
        | `Default(expr) => Defaults.evaluate(tryGet(resolve), expr)
        | `NotRegistered =>
          Log.warnf(m => m("Missing contributed default for `%s`", keyName));
          Some(Colors.magenta);
        };

      {
        key,
        defaults,
        tryFrom: resolve => tryGet(resolve, key),
        from: resolve =>
          tryGet(resolve, key)
          |> Option.value(~default=Colors.transparentWhite),
      };
    };
  };

  include DSL;
};

// COLORS

module Colors = {
  type t = Lookup.t(Color.t);

  let empty = Lookup.empty;
  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(((keyName, entry)) => (Lookup.key(keyName), entry))
    |> Lookup.of_seq;

  let get = Lookup.find_opt;
};

// THEME

type t = {
  variant,
  colors: Colors.t,
};
