type content('a) = {
  data: 'a,
  id: int,
  isOpen: bool,
  children: list(content('a)),
};

type fsNode = {
  displayName: string,
  depth: int,
  path: string,
  isDirectory: bool,
  icon: option(IconTheme.IconDefinition.t),
};

type t = content(fsNode);

type treeUpdate = {
  updated: t,
  tree: t,
};

let toggleNodeStatus = node => {...node, isOpen: !node.isOpen};

let updateNode = (nodeId, tree, ~updater=toggleNodeStatus, ()) => {
  let rec update = tree => {
    switch (tree) {
    | {id, _} as node when id == nodeId => updater(node)
    | node => {...node, children: List.map(update, node.children)}
    };
  };

  update(tree);
};
