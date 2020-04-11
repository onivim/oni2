[@deriving show]
type t('msg) = {
  id: string,
  title: option(string),
  category: option(string),
  icon: option([@opaque] IconTheme.IconDefinition.t),
  isEnabledWhen: WhenExpr.t,
  msg: [ | `Arg0('msg) | `Arg1(Json.t => 'msg)],
};

let map: ('a => 'b, t('a)) => t('b);

module Lookup: {
  type command('msg) = t('msg);
  type t('msg);

  let fromList: list(command('msg)) => t('msg);

  let get: (string, t('msg)) => option(command('msg));
  let add: (string, command('msg), t('msg)) => t('msg);

  let union: (t('msg), t('msg)) => t('msg);
  let unionMany: list(t('msg)) => t('msg);

  let map: ('a => 'b, t('a)) => t('b);

  let toList: t('msg) => list(command('msg));
};
