module type OrderedType = Map.OrderedType;

module type S = {
  type key;
  type path = list(key);
  module KeyedMap: Map.S with type key := key;
  type t('a) =
    | Node(KeyedMap.t(t('a)))
    | Leaf('a);

  let empty: t(_);

  let fromList: list((path, 'a)) => t('a);

  let add: (path, 'a, t('a)) => t('a);
  let get: (path, t('a)) => option('a);

  let merge:
    ((path, option('a), option('b)) => option('c), t('a), t('b)) => t('c);
  let union: ((path, 'a, 'a) => option('a), t('a), t('a)) => t('a);
  let map: ('a => 'b, t('a)) => t('b);
  let fold: ((path, 'a, 'b) => 'b, t('a), 'b) => 'b;
};

module Make: (Ord: OrderedType) => S with type key = Ord.t;
