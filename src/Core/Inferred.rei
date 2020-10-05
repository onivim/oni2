/**
 Inferred.rei

 A monad tracking 'implicit' vs 'explicit' settings.
*/

type t('a);

let implicit: 'a => t('a);
let explicit: 'a => t('a);

let isImplicit: t('a) => bool;
let isExplicit: t('a) => bool;

let map: ('a => 'b, t('a)) => t('b);
let flatMap: ('a => t('b), t('a)) => t('b);

let update: (~new_: t('a), t('a)) => t('a);

let value: t('a) => 'a;
