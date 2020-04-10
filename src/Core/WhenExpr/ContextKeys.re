module Log = (val Timber.Log.withNamespace("Oni2.Core.WhenExpr.ContextKeys"));

module Lookup = Kernel.KeyedStringMap;

module Value = {
  [@deriving show({with_path: false})]
  type t =
    | String(string)
    | True
    | False;

  // Emulate JavaScript semantics
  let asBool =
    fun
    | True => true
    | False => false
    | String("") => false
    | String(_) => true;

  // Emulate JavaScript semantics
  let asString =
    fun
    | True => "true"
    | False => "false"
    | String(str) => str;
};

module Schema = {
  type entry('model) = {
    key: string,
    get: 'model => Value.t,
  };

  let define = (key, get) => {key, get};
  let bool = (key, get) =>
    define(key, model => get(model) ? Value.True : Value.False);
  let string = (key, get) => define(key, model => Value.String(get(model)));

  type t('model) = Lookup.t(entry('model));

  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(entry => (Lookup.key(entry.key), entry))
    |> Lookup.of_seq;

  let union = (xs, ys) =>
    Lookup.union(
      (key, _x, y) => {
        Log.errorf(m =>
          m("Encountered duplicate context key: %s", Lookup.keyName(key))
        );
        Some(y);
      },
      xs,
      ys,
    );
  let unionMany = lookups => List.fold_left(union, Lookup.empty, lookups);

  let map = f =>
    Lookup.map(entry => {...entry, get: model => entry.get(f(model))});
};

type t = Lookup.t(Value.t);

let fromSchema = (schema, model) =>
  Lookup.map(Schema.(entry => entry.get(model)), schema);

let getValue = (lookup, key) =>
  Lookup.find_opt(Lookup.key(key), lookup)
  |> Option.value(~default=Value.False);
