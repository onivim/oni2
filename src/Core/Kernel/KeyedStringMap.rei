type key;
let key: string => key;
let keyName: key => string;

type t(+'a);

let empty: t('a);
let is_empty: t('a) => bool;

let mem: (key, t('a)) => bool;
let add: (key, 'a, t('a)) => t('a);
let update: (key, option('a) => option('a), t('a)) => t('a);
let remove: (key, t('a)) => t('a);
let merge:
  ((key, option('a), option('b)) => option('c), t('a), t('b)) => t('c);
let union: ((key, 'a, 'a) => option('a), t('a), t('a)) => t('a);

let compare: (('a, 'a) => int, t('a), t('a)) => int;
let equal: (('a, 'a) => bool, t('a), t('a)) => bool;

let iter: ((key, 'a) => unit, t('a)) => unit;
let fold: ((key, 'a, 'b) => 'b, t('a), 'b) => 'b;

let for_all: ((key, 'a) => bool, t('a)) => bool;
let exists: ((key, 'a) => bool, t('a)) => bool;

let filter: ((key, 'a) => bool, t('a)) => t('a);
let partition: ((key, 'a) => bool, t('a)) => (t('a), t('a));

let find_opt: (key, t('a)) => option('a);
let find_first_opt: (key => bool, t('a)) => option((key, 'a));
let find_last_opt: (key => bool, t('a)) => option((key, 'a));

let map: ('a => 'b, t('a)) => t('b);
let mapi: ((key, 'a) => 'b, t('a)) => t('b);

let to_seq: t('a) => Seq.t((key, 'a));
let add_seq: (Seq.t((key, 'a)), t('a)) => t('a);
let of_seq: Seq.t((key, 'a)) => t('a);
