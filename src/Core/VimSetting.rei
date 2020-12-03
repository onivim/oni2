type t =
  | String(string)
  | Int(int);

type decoder('a);

type fontDescription = {
  fontFamily: string,
  height: option(float),
};

module Schema: {
  let succeed: 'a => decoder('a);
  let fail: string => decoder('a);

  let string: decoder(string);
  let bool: decoder(bool);
  let int: decoder(int);

  let font: decoder(fontDescription);

  let map: ('a => 'b, decoder('a)) => decoder('b);
};

let decode_value: (decoder('a), t) => result('a, string);
let decode_value_opt: (decoder('a), t) => option('a);
