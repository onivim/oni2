type t('a);

let make: (~capacity: int, 'a) => t('a);

let push: ('a, t('a)) => unit;

let size: t('a) => int;

let capacity: t('a) => int;

let getAt: (int, t('a)) => 'a;
