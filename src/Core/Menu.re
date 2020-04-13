open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.Menu"));

// SCHEMA

module Schema = {
  [@deriving show]
  type item = {
    isVisibleWhen: WhenExpr.t,
    group: option(string),
    index: option(int),
    command: string,
    alt: option(string) // currently unused
  };

  type group = list(item);

  [@deriving show]
  type definition = {
    id: string,
    items: list(item),
  };

  let extend = (id, groups) => {id, items: List.concat(groups)};

  let group = (id, items) =>
    List.map(item => {...item, group: Some(id)}, items);

  let ungrouped = items => items;

  let item = (~index=?, ~alt=?, ~isVisibleWhen=WhenExpr.Value(True), command) => {
    isVisibleWhen,
    index,
    command,
    alt,
    group: None,
  };
};

// MODEL

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

// LOOKUP

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
