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

module Lookup: {
  type t;

  let fromList: list((string, list(item))) => t;

  let get: (string, t) => list(item);

  let union: (t, t) => t;
  let unionMany: list(t) => t;
};
