module Value: {
  [@deriving show]
  type t =
    | String(string)
    | True
    | False;

  let asBool: t => bool;
  let asString: t => string;
};

module ContextKeys: {
  module Schema: {
    type entry('model);
    type t('model);

    let define: (string, 'model => Value.t) => entry('model);
    let bool: (string, 'model => bool) => entry('model);
    let string: (string, 'model => string) => entry('model);

    let fromList: list(entry('model)) => t('model);

    let union: (t('model), t('model)) => t('model);
    let unionMany: list(t('model)) => t('model);

    let map: ('a => 'b, t('b)) => t('a);
  };

  type t;

  let fromList: list((string, Value.t)) => t;
  let fromSchema: (Schema.t('model), 'model) => t;

  let union: (t, t) => t;
  let unionMany: list(t) => t;

  let getValue: (t, string) => Value.t;
};

[@deriving show]
type t =
  | Defined(string)
  | Eq(string, Value.t)
  | Neq(string, Value.t)
  | Regex(string, option([@opaque] Oniguruma.OnigRegExp.t))
  | And(list(t))
  | Or(list(t))
  | Not(t)
  | Value(Value.t);

let evaluate: (t, string => Value.t) => bool;
let parse: string => t;
