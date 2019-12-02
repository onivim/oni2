type t = {
  id: int,
  path: string,
  displayName: string,
  icon: option(IconTheme.IconDefinition.t),
  depth: int,
  kind,
  expandedSubtreeSize: int,
}

and kind =
  | Directory({
      isOpen: bool,
      children: [ | `Loading | `Loaded(list(t))],
    })
  | File;

let rec _expandedSubtreeSize =
  fun
  | Directory({isOpen: true, children: `Loaded(children)}) =>
    List.fold_left(
      (acc, child) => acc + _expandedSubtreeSize(child.kind),
      1,
      children,
    )

  | _ => 1;

let create = (~id, ~path, ~icon, ~depth, ~kind) => {
  id,
  path,
  displayName: Filename.basename(path),
  icon,
  depth,
  kind,
  expandedSubtreeSize: _expandedSubtreeSize(kind),
};

let update = (~tree, ~updater, nodeId) => {
  let rec update = tree => {
    switch (tree) {
    | {id, _} as node when id == nodeId => updater(node)

    | {kind: Directory({children: `Loaded(children), _} as dir), _} as node => {
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
  fun
  | {kind: Directory({isOpen, children}), _} as node => {
      ...node,
      kind: Directory({isOpen: !isOpen, children}),
    }
  | node => node;

module Model = {
  type nonrec t = t;

  let children = node =>
    switch (node.kind) {
    | Directory({children, _}) => children
    | File => `Loaded([])
    };

  let kind = node =>
    switch (node.kind) {
    | Directory({isOpen: true, _}) => `Node(`Open)
    | Directory({isOpen: false, _}) => `Node(`Closed)
    | File => `Leaf
    };

  let expandedSubtreeSize = node => node.expandedSubtreeSize;
};
