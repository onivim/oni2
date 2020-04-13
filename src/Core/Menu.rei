// SCHEMA

module Schema: {
  [@deriving show]
  type item = {
    isVisibleWhen: WhenExpr.t,
    group: option(string),
    index: option(int),
    command: string,
    alt: option(string) // currently unused
  };

  type group;

  [@deriving show]
  type definition = {
    id: string,
    items: list(item),
  };

  let extend: (string, list(group)) => definition;

  let group: (string, list(item)) => group;
  let ungrouped: list(item) => group;

  let item:
    (~index: int=?, ~alt: string=?, ~isVisibleWhen: WhenExpr.t=?, string) =>
    item;
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

let fromSchemaItem: (Command.Lookup.t(_), Schema.item) => option(item);

// LOOKUP

module Lookup: {
  type t;

  let fromList: list((string, list(item))) => t;
  let fromSchema: (Command.Lookup.t(_), list(Schema.definition)) => t;

  let get: (string, t) => list(item);

  let union: (t, t) => t;
  let unionMany: list(t) => t;
};
