type t;

let empty: t;

let fromList: list((string, Json.t)) => t;
let fromFile: string => t;

let union: (t, t) => t;
let unionMany: list(t) => t;

module Schema: {
  type codec('a);
  type spec;
  type setting('a) = {
    spec,
    get: t => 'a,
  };

  let bool: codec(bool);
  let int: codec(int);
  let string: codec(string);
  let list: codec('a) => codec(list('a));

  let custom:
    (~decode: Json.decoder('a), ~encode: Json.encoder('a)) => codec('a);

  let setting: (string, codec('a), ~default: 'a) => setting('a);
};
