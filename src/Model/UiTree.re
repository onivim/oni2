type t = {
  id: int,
  path: string,
  displayName: string,
  icon: option(IconTheme.IconDefinition.t),
  depth: int,
  kind,
}

and kind =
  | Directory({
      isOpen: bool,
      children: [ | `Loading | `Loaded(list(t))],
    })
  | File;

let updateNode = (nodeId, tree, ~updater) => {
  let rec update = tree => {
    switch (tree) {
    | {id, _} as node when id == nodeId => updater(node)

    | {kind: Directory({children: `Loaded(children)} as dir)} as node => {
        ...node,
        kind:
          Directory({
            ...dir,
            children: `Loaded(List.map(update, children)),
          }),
      }

    | node => node
    };
  };

  update(tree);
};

let toggleOpenState =
  updateNode(
    ~updater=
      fun
      | {kind: Directory({isOpen, children})} as node => {
          ...node,
          kind: Directory({isOpen: !isOpen, children}),
        }
      | node => node,
  );

module Model = {
  type nonrec t = t;

  let children = node =>
    switch (node.kind) {
    | Directory({children}) => children
    | File => `Loaded([])
    };

  let kind = node =>
    switch (node.kind) {
    | Directory({isOpen: true, _}) => `Node(`Open)
    | Directory({isOpen: false, _}) => `Node(`Closed)
    | File => `Leaf
    };

  let rec expandedSubtreeSize = node =>
    switch (node.kind) {
    | Directory({isOpen: true, children: `Loaded(children)}) =>
      List.fold_left(
        (acc, child) => acc + expandedSubtreeSize(child),
        1,
        children,
      )

    | _ => 1
    };
};
