// Tree from Revery

type status =
  | Open
  | Closed;

type content('a) = {
  data: 'a,
  id: int,
  status,
};

/*
   A multiway tree, each node can have a variable number of siblings
   each of which can have a variable number of siblings etc.
   more info: https://ocaml.org/learn/tutorials/99problems.html#Multiway-Trees
 */
type tree('a) =
  | Empty
  | Node(content('a), list(tree('a)));

// End Tree

type fsNode('a) = {
  displayName: string,
  depth: int,
  path: string,
  isDirectory: bool,
  children: list('a),
  icon: option(IconTheme.IconDefinition.t),
  secondaryIcon: option(IconTheme.IconDefinition.t),
};

type treeItem =
  | FileSystemNode(fsNode(treeItem));

type t = tree(treeItem);
type itemContent = content(treeItem);

type treeUpdate = {
  updated: t,
  tree: t,
};

let toggleStatus = status =>
  switch (status) {
  | Open => Closed
  | Closed => Open
  };

let toggleNodeStatus = node => {
  switch (node) {
  | Node(content, children) =>
    Node({...content, status: toggleStatus(content.status)}, children)
  | Empty => Empty
  };
};

let updateNode = (nodeId, tree, ~updater=toggleNodeStatus, ()) => {
  let updatedNode = ref(Empty);

  let rec update = (nodeId, tree) => {
    switch (tree) {
    | Node({id, _}, _) as x when id == nodeId =>
      let node = updater(x);
      /*
       Store a reference to the located/updated node
       TODO: find a solution that doesn't require a ref
       */
      updatedNode := node;
      node;
    | Node(data, children) =>
      let newChildren = List.map(update(nodeId), children);
      Node(data, newChildren);
    | Empty => Empty
    };
  };

  {updated: updatedNode^, tree: update(nodeId, tree)};
};
