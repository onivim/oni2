[@deriving show]
type t('node, 'leaf) =
  | Leaf('leaf)
  | Node({
      expanded: bool,
      data: 'node,
      children: list(t('node, 'leaf)),
    });

let leaf = data => Leaf(data);

let node = (~expanded=true, ~children=[], data) => {
  Node({expanded, children, data});
};

let fold = (f, acc, tree) => {
  let rec loop = (current, node) => {
    let newAcc = f(current, node);
    switch (node) {
    | Leaf(_) => newAcc
    | Node({children, _}) => children |> List.fold_left(loop, newAcc)
    };
  };

  loop(acc, tree);
};

let map = (~leaf, ~node, tree) => {
  let rec loop = curr => {
    switch (curr) {
    | Leaf(data) => Leaf(leaf(data))
    | Node({children, data, _} as prev) =>
      let newChildren = children |> List.map(loop);

      let data = node(data);
      Node({...prev, children: newChildren, data});
    };
  };

  loop(tree);
};

let setExpanded = (f, tree) => {
  let rec loop = node => {
    switch (node) {
    | Leaf(_) as leaf => leaf
    | Node({children, data, expanded, _} as node) =>
      let newChildren = children |> List.map(loop);

      let expanded = f(~current=expanded, data);
      Node({...node, children: newChildren, expanded});
    };
  };

  loop(tree);
};
