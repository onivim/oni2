module Key: {
  type t =
    pri {
      hash: int,
      name: string,
    };
  let compare: (t, t) => int;
  let create: string => t;
} = {
  type t = {
    hash: int,
    name: string,
  };

  let compare = (a, b) =>
    a.hash == b.hash ? compare(a.name, b.name) : compare(a.hash, b.hash);

  let create = name => {hash: Hashtbl.hash(name), name};
};

include Map.Make(Key);

let key = Key.create;
let keyName = (key: key) => key.name;
