type key;
type t('a);

let key: string => key;
let keyName: key => string;

let empty: t(_);

let fromList: list((key, 'a)) => t('a);

let add: (key, 'a, t('a)) => t('a);
let update: (key, option('a), t('a)) => t('a);
let get: (key, t('a)) => option('a);

let union: ((key, 'a, 'a) => option('a), t('a), t('a)) => t('a);
let map: ('a => 'b, t('a)) => t('b);
