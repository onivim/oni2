type t;

let empty: t;

let fromList: list((string, Json.t)) => t;
let fromFile: string => t;

let union: (t, t) => t;
let unionMany: list(t) => t;

module Schema: {
  type decoder('a);
  type setting('a) = {get: t => 'a};

  let bool: decoder(bool);
  let int: decoder(int);
  let string: decoder(string);
  let list: decoder('a) => decoder(list('a));

  let custom: Json.decoder('a) => decoder('a);

  let setting: (string, decoder('a), ~default: 'a) => setting('a);
};
