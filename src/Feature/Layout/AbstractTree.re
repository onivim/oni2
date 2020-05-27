[@deriving show({with_path: false})]
type node('id, 'meta) = {
  meta: 'meta,
  kind: [
    | `Split([ | `Horizontal | `Vertical], list(node('id, 'meta)))
    | `Window('id)
  ],
};

module DSL = {
  let split = (~meta, direction, children) => {
    meta,
    kind: `Split((direction, children)),
  };
  let vsplit = (~meta, children) => split(~meta, `Vertical, children);
  let hsplit = (~meta, children) => split(~meta, `Horizontal, children);
  let window = (~meta, id) => {meta, kind: `Window(id)};

  let withMetadata = (meta, node) => {...node, meta};
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

let%test_module "rotate" =
  (module
   {
     module DSL = {
       include DSL;
       let hsplit = hsplit(~meta=());
       let vsplit = vsplit(~meta=());
       let window = window(~meta=());
     };
     open DSL;

     let%test "forward - simple tree" = {
       let initial = vsplit([window(3), window(2), window(1)]);

       let actual = rotate(`Forward, 3, initial);

       actual == vsplit([window(1), window(3), window(2)]);
     };

     let%test "forward - nested tree" = {
       let initial =
         vsplit([window(4), hsplit([window(3), window(2), window(1)])]);

       let actual = rotate(`Forward, 1, initial);

       actual
       == vsplit([window(4), hsplit([window(1), window(3), window(2)])]);
     };

     let%test "backward - simple tree" = {
       let initial = vsplit([window(3), window(2), window(1)]);

       let actual = rotate(`Backward, 3, initial);

       actual == vsplit([window(2), window(1), window(3)]);
     };

     let%test "backward - nested tree" = {
       let initial =
         vsplit([window(4), hsplit([window(3), window(2), window(1)])]);

       let actual = rotate(`Backward, 1, initial);

       actual
       == vsplit([window(4), hsplit([window(2), window(1), window(3)])]);
     };
   });
