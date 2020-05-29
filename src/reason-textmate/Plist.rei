[@deriving show]
type t =
  | True
  | False
  | String(string)
  | Integer(int)
  | Real(float)
  // Date
  // Data
  | Array(list(t))
  | Dict(list((string, t)));

type decoder('a) = t => result('a, string);

let id: decoder(t);

let bool: decoder(bool);
let string: decoder(string);
let integer: decoder(int);
let real: decoder(float);
let array: decoder('a) => decoder(list('a));

type dictGetters = {
  required: 'a. (string, decoder('a)) => 'a,
  optional: 'a. (string, decoder('a)) => option('a),
  withDefault: 'a. (string, decoder('a), 'a) => 'a,
};
let dict: (dictGetters => 'b) => decoder('b);
let assoc: decoder('a) => decoder(list((string, 'a)));
let property: (string, decoder('a)) => decoder('a);

let option: decoder('a) => decoder(option('a));

let oneOf: list((string, decoder('a))) => decoder('a);
let map: ('a => 'b, decoder('a)) => decoder('b);
