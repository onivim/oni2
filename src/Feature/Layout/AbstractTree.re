type node('id, 'meta) = {
  meta: 'meta,
  kind: [
    | `Split([ | `Horizontal | `Vertical], list(node('id, 'meta)))
    | `Window('id)
  ],
};

let rec fold = (f, acc, node) =>
  switch (node.kind) {
  | `Window(_) => f(node, acc)
  | `Split(_, children) => List.fold_left(fold(f), acc, children)
  };

let rec map = (f, node) =>
  switch (node.kind) {
  | `Window(_) => f(node)
  | `Split(direction, children) =>
    f({...node, kind: `Split((direction, List.map(map(f), children)))})
  };

let rec windowNodes = node =>
  switch (node.kind) {
  | `Window(_) => [node]
  | `Split(_, children) => children |> List.map(windowNodes) |> List.concat
  };

let rec windows = node =>
  switch (node.kind) {
  | `Window(id) => [id]
  | `Split(_, children) => children |> List.map(windows) |> List.concat
  };

let rec rotate = (direction, targetId, node) => {
  let rotateList =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail |> List.rev |> List.cons(head) |> List.rev;

  switch (node.kind) {
  | `Split(dir, children) =>
    let children =
      if (List.exists(child => child.kind == `Window(targetId), children)) {
        switch (direction) {
        | `Backward => children |> rotateList
        | `Forward => children |> List.rev |> rotateList |> List.rev
        };
      } else {
        List.map(rotate(direction, targetId), children);
      };

    {...node, kind: `Split((dir, children))};

  | `Window(_) => node
  };
};
