open Oni_Core;
open Utility;

[@deriving show({with_path: false})]
type metadata = {
  path: string,
  displayName: string,
  hash: int // hash of basename, so only comparable locally
};

[@deriving show({with_path: false})]
type t = Tree.t(metadata, metadata);

let _hash = Hashtbl.hash;
let _pathHashes = (~base, path) =>
  path |> Path.toRelative(~base) |> Path.explode |> List.map(_hash);

let file = path => {
  let basename = Filename.basename(path);

  Tree.leaf({path, hash: _hash(basename), displayName: basename});
};

let directory = (~isOpen=false, path, ~children) => {
  let basename = Filename.basename(path);

  Tree.node(
    ~expanded=isOpen,
    ~children,
    {path, hash: _hash(basename), displayName: basename},
  );
};

let get = f =>
  fun
  | Tree.Node({data, _})
  | Tree.Leaf(data) => f(data);

let getHash = get(({hash, _}) => hash);
let getPath = get(({path, _}) => path);
let displayName = get(({displayName, _}) => displayName);

let equals = (a, b) =>
  getHash(a) == getHash(b) && getPath(a) == getPath(b);

let findNodesByPath = (path, tree) => {
  let rec loop = (focusedNodes, children, pathHashes) =>
    switch (pathHashes) {
    | [] => `Success(List.rev(focusedNodes))
    | [hash, ...rest] =>
      switch (children) {
      | [] =>
        let last = List.hd(focusedNodes);
        if (equals(last, tree)) {
          `Failed;
        } else {
          `Partial(last);
        };

      | [node, ...children] =>
        if (getHash(node) == hash) {
          let children = Tree.children(node);
          loop([node, ...focusedNodes], children, rest);
        } else {
          loop(focusedNodes, children, pathHashes);
        }
      }
    };

  switch (tree) {
  | Node({children, _}) =>
    loop([tree], children, _pathHashes(~base=getPath(tree), path))
  | Leaf(_) => `Failed
  };
};

let findByPath = (path, tree) => {
  let rec loop = (node, children, pathHashes) =>
    switch (pathHashes) {
    | [] => Some(node)
    | [hash, ...rest] =>
      switch (children) {
      | [] => None

      | [node, ...children] =>
        if (getHash(node) == hash) {
          let children = Tree.children(node);
          loop(node, children, rest);
        } else {
          loop(node, children, pathHashes);
        }
      }
    };

  switch (tree) {
  | Tree.Node({children, _}) =>
    loop(tree, children, _pathHashes(~base=getPath(tree), path))
  | Tree.Leaf(_) => None
  };
};

let replace = (~replacement, tree) => {
  let rec loop = (pathHashes, node) =>
    switch (pathHashes) {
    | [hash] when hash == getHash(node) => replacement

    | [hash, ...rest] when hash == getHash(node) =>
      switch (node) {
      | Tree.Node({children, _} as dir) =>
        let newChildren = List.map(loop(rest), children);
        Tree.Node({...dir, children: newChildren});
      | Tree.Leaf(_) => node
      }

    | _ => node
    };

  loop(
    _pathHashes(
      ~base=Filename.dirname(getPath(tree)),
      getPath(replacement),
    ),
    tree,
  );
};

let updateNodesInPath =
    (
      updater: Tree.t(metadata, metadata) => Tree.t(metadata, metadata),
      path,
      tree,
    ) => {
  let rec loop = (pathHashes, node: Tree.t(metadata, metadata)) =>
    switch (pathHashes) {
    | [hash, ...rest] when hash == getHash(node) =>
      switch (node) {
      | Tree.Node({children, _}) as cur =>
        let newChildren = List.map(loop(rest), children);
        switch (updater(cur)) {
        | Tree.Node(data) => Tree.Node({...data, children: newChildren})
        | Tree.Leaf(_) as leaf => leaf
        };

      | Tree.Leaf(_) as leaf => updater(leaf)
      }

    | _ => node
    };

  loop(_pathHashes(~base=Filename.dirname(getPath(tree)), path), tree);
};

let toggleOpen =
  fun
  | Tree.Node({expanded: prev, _} as node) =>
    Tree.Node({...node, expanded: !prev})
  | leaf => leaf;

let setOpen =
  fun
  | Tree.Node(node) => Tree.Node({...node, expanded: true})
  | leaf => leaf;

module Model = {
  type nonrec t = t;

  let children = node =>
    switch (node) {
    | Tree.Node({children, _}) => children
    | Tree.Leaf(_) => []
    };

  let kind = node =>
    switch (node) {
    | Tree.Node({expanded: true, _}) => `Node(`Open)
    | Tree.Node({expanded: false, _}) => `Node(`Closed)
    | Tree.Leaf(_) => `Leaf
    };
};
