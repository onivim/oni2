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
};

module Make = (Ord: OrderedType) => {
  type key = Ord.t;
  type path = list(key);
  module KeyedMap = Map.Make(Ord);
  type t('a) = KeyedMap.t(node('a))
  and node('a) = {
    value: option('a),
    children: t('a),
  };

  let empty = KeyedMap.empty;

  let rec update = (path, value, tree) => {
    let updateEntry = (key, f) =>
      KeyedMap.update(key, node => Some(f(node)), tree);
    let setNodeValue = (key, value) =>
      updateEntry(
        key,
        fun
        | Some(node) => {...node, value}
        | None => {value, children: empty},
      );
    let updateChildren = (key, f) =>
      updateEntry(
        key,
        fun
        | Some(node) => {...node, children: f(node.children)}
        | None => {value: None, children: f(empty)},
      );

    switch (path) {
    | [] => tree
    | [key] => setNodeValue(key, value)
    | [key, ...rest] => updateChildren(key, update(rest, value))
    };
  };

  let add = (keys, value) => update(keys, Some(value));

  let fromList = entries =>
    List.fold_left(
      (acc, (key, value)) => add(key, value, acc),
      empty,
      entries,
    );

  let rec get = (path, tree) =>
    switch (path) {
    | [] => None
    | [key] =>
      let maybeNode = KeyedMap.find_opt(key, tree);
      Option.bind(maybeNode, node => node.value);
    | [key, ...rest] =>
      let maybeNode = KeyedMap.find_opt(key, tree);
      Option.bind(maybeNode, node => get(rest, node.children));
    };

  let union = f => {
    let rec aux = (f, path) =>
      KeyedMap.union((key, a, b) => {
        let path = [key, ...path];
        let value =
          switch (a.value, b.value) {
          | (None, None) => None
          | (a, None) => a
          | (None, b) => b
          | (Some(a), Some(b)) => f(List.rev(path), a, b)
          };

        Some({value, children: aux(f, path, a.children, b.children)});
      });

    aux(f, []);
  };

  let rec map = f =>
    KeyedMap.map(node => {
      {value: Option.map(f, node.value), children: map(f, node.children)}
    });
};
