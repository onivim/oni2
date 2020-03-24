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

module Make = (Ord: OrderedType) => {
  type key = Ord.t;
  type path = list(key);
  module KeyedMap = Map.Make(Ord);
  type t('a) =
    | Node(KeyedMap.t(t('a)))
    | Leaf('a);

  let empty = Node(KeyedMap.empty);

  let rec update = (path, value: option('a), node: t('a)) => {
    switch (path) {
    | [] => node

    | [key] =>
      switch (node) {
      | Node(children) =>
        Node(
          KeyedMap.update(
            key,
            fun
            | None => Option.map(value => Leaf(value), value)
            | Some(Leaf(_)) => Option.map(value => Leaf(value), value) // duplicate, override existing value
            | Some(Node(_)) as node => node, // ignore if internal node, NOTE: lossy
            children,
          ),
        )
      | Leaf(_) =>
        switch (value) {
        | Some(value) => Node(KeyedMap.singleton(key, Leaf(value)))
        | None => Node(KeyedMap.empty)
        }
      }

    | [key, ...rest] =>
      switch (node) {
      | Node(children) =>
        let newChildren =
          KeyedMap.update(
            key,
            fun
            | Some(Node(_) as child) => Some(update(rest, value, child))
            | None
            | Some(Leaf(_)) => Some(update(rest, value, empty)), // override leaf, NOTE: lossy
            children,
          );

        Node(newChildren);

      | Leaf(_) =>
        // override leaf, NOTE: lossy
        let child = update(rest, value, empty);
        Node(KeyedMap.singleton(key, child));
      }
    };
  };

  let add = (keys, value) => update(keys, Some(value));

  let fromList = entries =>
    List.fold_left(
      (acc, (key, value)) => add(key, value, acc),
      empty,
      entries,
    );

  let rec get = (path, node) =>
    switch (path, node) {
    | ([], Leaf(value)) => Some(value)
    | ([key, ...rest], Node(children)) =>
      switch (KeyedMap.find_opt(key, children)) {
      | Some(child) => get(rest, child)
      | None => None
      }
    | _ => None
    };

  let merge = (f, a, b) => {
    let f = (path, a, b) =>
      Option.map(value => Leaf(value), f(path, a, b));

    let rec aux = (path, a, b) =>
      switch (a, b) {
      | (Leaf(a), Leaf(b)) => f(path, Some(a), Some(b))
      | (Node(_), Leaf(b)) => f(path, None, Some(b))
      | (Leaf(a), Node(_)) => f(path, Some(a), None)
      | (Node(aChildren), Node(bChildren)) =>
        let merged =
          KeyedMap.merge(
            (key, a, b) => {
              let path = [key, ...path];
              switch (a, b) {
              | (Some(a), Some(b)) => aux(path, a, b)
              | (None, Some(b)) => aux(path, empty, b)
              | (Some(a), None) => aux(path, a, empty)
              | (None, None) => failwith("unreachable")
              };
            },
            aChildren,
            bChildren,
          );

        if (merged == KeyedMap.empty) {
          None;
        } else {
          Some(Node(merged));
        };
      };

    aux([], a, b) |> Option.value(~default=empty);
  };

  let union = f => {
    let rec aux = (f, path, a, b) =>
      switch (a, b) {
      | (Leaf(_), Leaf(b)) => Leaf(b) // duplicate, override a with b
      | (Node(_), Leaf(_)) => a // Node always wins, NOTE: lossy
      | (Leaf(_), Node(_)) => b // Node always wins, NOTE: lossy
      | (Node(aChildren), Node(bChildren)) =>
        // merge children
        let merged =
          KeyedMap.union(
            (key, a, b) => {
              let path = [key, ...path];
              Some(aux(f, path, a, b));
            },
            aChildren,
            bChildren,
          );
        Node(merged);
      };

    aux(f, []);
  };

  let rec map = f =>
    fun
    | Leaf(value) => Leaf(f(value))
    | Node(children) => Node(KeyedMap.map(map(f), children));

  let fold = (f, node, initial) => {
    let rec traverse = (node, revPath, acc) =>
      switch (node) {
      | Leaf(value) => f(List.rev(revPath), value, acc)
      | Node(children) =>
        KeyedMap.fold(
          (key, node, acc) => traverse(node, [key, ...revPath], acc),
          children,
          acc,
        )
      };

    traverse(node, [], initial);
  };
};
