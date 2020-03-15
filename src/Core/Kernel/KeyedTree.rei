module type OrderedType = Map.OrderedType;

module type S = {
  type key;
  type path = list(key);
  type t('a);

  let empty: t(_);

  let fromList: list((path, 'a)) => t('a);

  let add: (path, 'a, t('a)) => t('a);
  let update: (path, option('a), t('a)) => t('a);
  let get: (path, t('a)) => option('a);

  let union: ((path, 'a, 'a) => option('a), t('a), t('a)) => t('a);
  let map: ('a => 'b, t('a)) => t('b);
  let fold: ((path, 'a, 'b) => 'b, t('a), 'b) => 'b;
};

module Make: (Ord: OrderedType) => S with type key = Ord.t;
