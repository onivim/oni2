type t;

let empty: t;

let fromList: list((string, Json.t)) => t;

let union: (t, t) => t;
let unionMany: list(t) => t;

module Schema: {
  type decoder('a);
  type setting('a) = t => 'a;

  let bool: decoder(option(bool));
  let int: decoder(option(int));
  let string: decoder(option(string));
  let list: decoder(option('a)) => decoder(option(list('a)));

  let default: ('a, decoder(option('a))) => decoder('a);
  let custom: Json.decoder('a) => decoder(option('a));

  let setting: (string, decoder('a)) => setting('a);
};
