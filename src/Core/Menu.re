open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.Menu"));

[@deriving show]
type item = {
  label: string,
  category: option(string),
  icon: [@opaque] option(IconTheme.IconDefinition.t),
  isEnabledWhen: [@opaque] WhenExpr.t,
  isVisibleWhen: [@opaque] WhenExpr.t,
  group: option(string),
  index: option(int),
  command: string,
};

module Lookup = {
  type t = KeyedStringMap.t(list(item));

  let fromList = entries =>
    entries
    |> List.to_seq
    |> Seq.map(((id, items)) => (KeyedStringMap.key(id), items))
    |> KeyedStringMap.of_seq;

  let get = (id, lookup) =>
    KeyedStringMap.find_opt(KeyedStringMap.key(id), lookup)
    |> Option.value(~default=[]);

  let union = (xs, ys) =>
    KeyedStringMap.union((_key, x, y) => Some(x @ y), xs, ys);

  let unionMany = lookups =>
    List.fold_left(union, KeyedStringMap.empty, lookups);
};
