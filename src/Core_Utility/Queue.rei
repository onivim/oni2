module type S = {
  type t('a);

  let empty: t(_);
  let length: t('a) => int;
  let isEmpty: t('a) => bool;
  let push: ('a, t('a)) => t('a);
  let pushFront: ('a, t('a)) => t('a);
  let pop: t('a) => (option('a), t('a));
  let take: (int, t('a)) => (list('a), t('a));
  let toList: t('a) => list('a);
};

include S;
