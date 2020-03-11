type key = list(string);
type t('a) = StringMap.t(node('a))
and node('a) = {
  value: option('a),
  children: t('a),
};

let key = String.split_on_char('.');
let keyName = String.concat(".");

let empty = StringMap.empty;

let rec update = (keys, value, tree) => {
  let updateEntry = (key, f) =>
    StringMap.update(key, node => Some(f(node)), tree);
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

  switch (keys) {
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

let rec get = (keys, tree) =>
  switch (keys) {
  | [] => None
  | [key] =>
    let maybeNode = StringMap.find_opt(key, tree);
    Option.bind(maybeNode, node => node.value);
  | [key, ...rest] =>
    let maybeNode = StringMap.find_opt(key, tree);
    Option.bind(maybeNode, node => get(rest, node.children));
  };

let union = f => {
  let rec aux = (f, path) =>
    StringMap.union((key, a, b) => {
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
  StringMap.map(node => {
    {value: Option.map(f, node.value), children: map(f, node.children)}
  });
