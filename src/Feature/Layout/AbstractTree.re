open Utility;

type node('id, 'meta) = {
  meta: 'meta,
  kind: [
    | `Split([ | `Horizontal | `Vertical], list(node('id, 'meta)))
    | `Window('id)
  ],
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

let rec rotate = (targetId, f, node) => {
  switch (node.kind) {
  | `Split(direction, children) =>
    let children =
      List.exists(({kind, _}) => kind == `Window(targetId), children)
        ? f(children) : List.map(rotate(targetId, f), children);

    {...node, kind: `Split((direction, children))};

  | `Window(_) => node
  };
};

let rotateForward = (target, tree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | list =>
      switch (ListEx.last(list)) {
      | Some(x) => [x, ...ListEx.dropLast(list)]
      | None => []
      };

  rotate(target, f, tree);
};

let rotateBackward = (target, tree) => {
  let f =
    fun
    | [] => []
    | [a] => [a]
    | [a, b] => [b, a]
    | [head, ...tail] => tail @ [head];

  rotate(target, f, tree);
};
