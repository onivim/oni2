type t = {
  id: int,
  path: string,
  displayName: string,
  icon: option(IconTheme.IconDefinition.t),
  kind,
  expandedSubtreeSize: int,
}

and kind =
  | Directory({
      isOpen: bool,
      children: [ | `Loading | `Loaded(list(t))],
    })
  | File;

let rec countExpandedSubtree =
  fun
  | Directory({isOpen: true, children: `Loaded(children)}) =>
    List.fold_left(
      (acc, child) => acc + countExpandedSubtree(child.kind),
      1,
      children,
    )

  | _ => 1;

let file = (path, ~id, ~icon) => {
  id,
  path,
  displayName: Filename.basename(path),
  icon,
  kind: File,
  expandedSubtreeSize: 1,
};

let directory = (~isOpen=false, path, ~id, ~icon, ~children) => {
  let kind = Directory({isOpen, children});

  {
    id,
    path,
    displayName: Filename.basename(path),
    icon,
    kind,
    expandedSubtreeSize: countExpandedSubtree(kind),
  };
};

let update = (~tree, ~updater, nodeId) => {
  let rec update = tree => {
    switch (tree) {
    | {id, _} as node when id == nodeId => updater(node)

    | {kind: Directory({children: `Loaded(children), _} as dir), _} as node =>
      let kind =
        Directory({...dir, children: `Loaded(List.map(update, children))});

      {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};

    | node => node
    };
  };

  update(tree);
};

let toggleOpenState =
  fun
  | {kind: Directory({isOpen, children}), _} as node => {
      let kind = Directory({isOpen: !isOpen, children});

      {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};
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
