open Oni_Core;

[@deriving show({with_path: false})]
type metadata = {
  path: [@opaque] FpExp.t(FpExp.absolute),
  displayName: string,
  hash: int, // hash of basename, so only comparable locally
  isSymlink: bool,
};

[@deriving show({with_path: false})]
type t = Tree.t(metadata, metadata);

module PathHasher = {
  let hash = Hashtbl.hash;

  // type t = list(string);

  let make = (~base, path) => {
    switch (FpExp.relativize(~source=base, ~dest=path)) {
    | Ok(relativePath) => FpExp.explode(relativePath) |> List.map(hash)
    | Error(_) => []
    };
  };

  let%test "equivalent paths" = {
    // TODO: Is this case even correct?
    make(~base=FpExp.(root), FpExp.(root)) == [];
  };

  let%test "simple path" = {
    make(~base=FpExp.(root), FpExp.(At.(root / "abc"))) == [hash("abc")];
  };

  let%test "multiple paths" = {
    make(~base=FpExp.(root), FpExp.(At.(root / "abc" / "def")))
    == [hash("abc"), hash("def")];
  };
};

let file = (~isSymlink, path) => {
  let basename = FpExp.baseName(path) |> Option.value(~default="(empty)");

  Tree.leaf({
    isSymlink,
    path,
    hash: PathHasher.hash(basename),
    displayName: basename,
  });
};

let directory = (~isOpen=false, ~isSymlink, path, ~children) => {
  let basename = FpExp.baseName(path) |> Option.value(~default="(empty)");

  Tree.node(
    ~expanded=isOpen,
    ~children,
    {
      isSymlink,
      path,
      hash: PathHasher.hash(basename),
      displayName: basename,
    },
  );
};

let get = f =>
  fun
  | Tree.Node({data, _})
  | Tree.Leaf(data) => f(data);

let isSymlink = get(({isSymlink, _}) => isSymlink);

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
    loop([tree], children, PathHasher.make(~base=getPath(tree), path))
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
    loop(tree, children, PathHasher.make(~base=getPath(tree), path))
  | Tree.Leaf(_) => None
  };
};

let replace = (~replacement, tree) => {
  let merge = (originalNode, newNode) =>
    switch (originalNode, newNode) {
    | (Tree.Leaf(_), _) => newNode
    | (_, Tree.Leaf(_)) => newNode
    // Merge the 'old' children with the 'new' children, so that we don't have to recursively
    // refresh if we don't have to.
    | (
        Tree.Node({children: oldChildren, expanded, data: prevData, _}),
        Tree.Node({children: newChildren, data: newData, _}),
      ) =>
      // Grab a map of previous children. For any that were existing, use the old metadata - some children might be expanded, for example.
      let previousMap =
        oldChildren
        |> List.fold_left(
             (
               acc: IntMap.t(Tree.t(metadata, metadata)),
               child: Tree.t(metadata, metadata),
             ) => {
               switch (child) {
               | Tree.Leaf(_) => acc
               | Tree.Node({data, _}) => IntMap.add(data.hash, child, acc)
               }
             },
             IntMap.empty,
           );

      // Now, go through the new children - see if any were around previously.
      // If so, use the previous tree.

      let children =
        newChildren
        |> List.map(child => {
             switch (child) {
             | Tree.Leaf(_) => child
             | Tree.Node({data, _}) =>
               IntMap.find_opt(data.hash, previousMap)
               |> Option.value(~default=child)
             }
           });

      // We assume that the 'symlink' state doesn't change when re-loading...
      let data = {...newData, isSymlink: prevData.isSymlink};

      Tree.Node({children, expanded, data});
    };

  let rec loop = (pathHashes, node) =>
    switch (pathHashes) {
    | [hash] when hash == getHash(node) => merge(node, replacement)

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
    PathHasher.make(
      ~base=FpExp.dirName(getPath(tree)),
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

  loop(PathHasher.make(~base=FpExp.dirName(getPath(tree)), path), tree);
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
