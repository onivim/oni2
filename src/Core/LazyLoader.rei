/*
 * LazyLoader.re
 */

type loadFunction('a) = string => result('a, string);

type t('a);

let create: loadFunction('a) => t('a);

let get: (t('a), string) => result('a, string);

// [fail] is a [LazyLoader.t] that always fails.
let fail: t(unit);

// [success(v)] returns a [LazyLoader.t]
// that returns [v] for every key.
let success: 'a => t('a);
