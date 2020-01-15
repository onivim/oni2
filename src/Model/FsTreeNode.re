open Oni_Core;

module ArrayEx = Utility.ArrayEx;
module Path = Utility.Path;

[@deriving show({with_path: false})]
type t = {
  path: string,
  displayName: string,
  hash: int, // hash of basename, so only comparable locally
  icon: option([@opaque] IconTheme.IconDefinition.t),
  kind,
  expandedSubtreeSize: int,
}

and kind =
  | Directory({
      isOpen: bool,
      children: [@opaque] list(t),
    })
  | File;

let _hash = Hashtbl.hash;
let _pathHashes = (~base, path) =>
  path |> Path.toRelative(~base) |> Path.explode |> List.map(_hash);

let rec countExpandedSubtree =
  fun
  | Directory({isOpen: true, children}) =>
    List.fold_left(
      (acc, child) => acc + countExpandedSubtree(child.kind),
      1,
      children,
    )

  | _ => 1;

let file = (path, ~icon) => {
  let basename = Filename.basename(path);

  {
    path,
    hash: _hash(basename),
    displayName: basename,
    icon,
    kind: File,
    expandedSubtreeSize: 1,
  };
};

let directory = (~isOpen=false, path, ~icon, ~children) => {
  let kind = Directory({isOpen, children});
  let basename = Filename.basename(path);

  {
    path,
    hash: _hash(basename),
    displayName: basename,
    icon,
    kind,
    expandedSubtreeSize: countExpandedSubtree(kind),
  };
};

let equals = (a, b) => a.hash == b.hash && a.path == b.path;

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
        if (node.hash == hash) {
          let children =
            switch (node.kind) {
            | Directory({children, _}) => children
            | File => []
            };
          loop([node, ...focusedNodes], children, rest);
        } else {
          loop(focusedNodes, children, pathHashes);
        }
      }
    };

  switch (tree.kind) {
  | Directory({children, _}) =>
    loop([tree], children, _pathHashes(~base=tree.path, path))
  | File => `Failed
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
        if (node.hash == hash) {
          let children =
            switch (node.kind) {
            | Directory({children, _}) => children
            | File => []
            };
          loop(node, children, rest);
        } else {
          loop(node, children, pathHashes);
        }
      }
    };

  switch (tree.kind) {
  | Directory({children, _}) =>
    loop(tree, children, _pathHashes(~base=tree.path, path))
  | File => None
  };
};

let prevExpandedNode = (path, tree) =>
  switch (findNodesByPath(path, tree)) {
  | `Success(nodePath) =>
    switch (List.rev(nodePath)) {
    // Has a parent, and therefore also siblings
    | [focus, {kind: Directory({children, _}), _} as parent, ..._] =>
      let children = children |> Array.of_list;
      let index = children |> ArrayEx.findIndex(equals(focus));

      switch (index) {
      // is first child
      | Some(index) when index == 0 => Some(parent)

      | Some(index) =>
        switch (children[index - 1]) {
        // is open directory with at least one child
        | {
            kind: Directory({isOpen: true, children: [_, ..._] as children}),
            _,
          } =>
          let children = children |> Array.of_list;
          let lastChild = children[Array.length(children) - 1];
          Some(lastChild);

        | prev => Some(prev)
        }

      | None => None // is not a child of its parent (?!)
      };
    | _ => None // has neither parent or siblings
    }
  | `Partial(_)
  | `Failed => None // path does not exist in this ree
  };

let nextExpandedNode = (path, tree) =>
  switch (findNodesByPath(path, tree)) {
  | `Success(nodePath) =>
    let rec loop = revNodePath =>
      switch (revNodePath) {
      | [
          focus,
          ...[{kind: Directory({children, _}), _}, ..._] as ancestors,
        ] =>
        let children = children |> Array.of_list;
        let index = children |> ArrayEx.findIndex(equals(focus));

        switch (index) {
        // is last child
        | Some(index) when index == Array.length(children) - 1 =>
          loop(ancestors)

        | Some(index) =>
          let next = children[index + 1];
          Some(next);

        | None => None // is not a child of its parent (?!)
        };
      | _ => None // has neither parent or siblings
      };

    switch (List.rev(nodePath)) {
    // Focus is open directory with at least one child
    | [
        {kind: Directory({isOpen: true, children: [firstChild, ..._]}), _},
        ..._,
      ] =>
      Some(firstChild)
    | revNodePath => loop(revNodePath)
    };

  | `Partial(_)
  | `Failed => None // path does not exist in this tree
  };

// Counts the number of expanded nodes before the node specified by the given path
let expandedIndex = (path, tree) => {
  let rec loop = (node, pathHashes) =>
    switch (pathHashes) {
    | [] => `Found(0)
    | [hash, ...pathTail] =>
      if (node.hash != hash) {
        `NotFound(node.expandedSubtreeSize);
      } else {
        switch (node.kind) {
        | Directory({isOpen: false, _})
        | File => `Found(0)

        | Directory({isOpen: true, children}) =>
          let rec loopChildren = (count, children) =>
            switch (children) {
            | [] => `NotFound(count)
            | [child, ...childTail] =>
              switch (loop(child, pathTail)) {
              | `Found(subtreeCount) => `Found(count + subtreeCount)
              | `NotFound(subtreeCount) =>
                loopChildren(count + subtreeCount, childTail)
              }
            };

          loopChildren(1, children);
        };
      }
    };

  switch (loop(tree, _pathHashes(~base=Filename.dirname(tree.path), path))) {
  | `Found(count) => Some(count)
  | `NotFound(_) => None
  };
};

let replace = (~replacement, tree) => {
  let rec loop = (pathHashes, node) =>
    switch (pathHashes) {
    | [hash] when hash == node.hash => replacement

    | [hash, ...rest] when hash == node.hash =>
      switch (node.kind) {
      | Directory({children, _} as dir) =>
        let newChildren = List.map(loop(rest), children);
        let kind = Directory({...dir, children: newChildren});

        {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};

      | File => node
      }

    | _ => node
    };

  loop(
    _pathHashes(~base=Filename.dirname(tree.path), replacement.path),
    tree,
  );
};

let updateNodesInPath = (updater, path, tree) => {
  let rec loop = (pathHashes, node) =>
    switch (pathHashes) {
    | [hash, ...rest] when hash == node.hash =>
      switch (node.kind) {
      | Directory({children, _} as dir) =>
        let newChildren = List.map(loop(rest), children);
        let newNode =
          updater({
            ...node,
            kind: Directory({...dir, children: newChildren}),
          });

        {
          ...newNode,
          expandedSubtreeSize: countExpandedSubtree(newNode.kind),
        };

      | File => updater(node)
      }

    | _ => node
    };

  loop(_pathHashes(~base=Filename.dirname(tree.path), path), tree);
};

let toggleOpen =
  fun
  | {kind: Directory({isOpen, children}), _} as node => {
      let kind = Directory({isOpen: !isOpen, children});

      {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};
    }
  | node => node;

let setOpen =
  fun
  | {kind: Directory({children, _}), _} as node => {
      let kind = Directory({isOpen: true, children});

      {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};
    }
  | node => node;

module Model = {
  type nonrec t = t;

  let children = node =>
    switch (node.kind) {
    | Directory({children, _}) => children
    | File => []
    };

  let kind = node =>
    switch (node.kind) {
    | Directory({isOpen: true, _}) => `Node(`Open)
    | Directory({isOpen: false, _}) => `Node(`Closed)
    | File => `Leaf
    };

  let expandedSubtreeSize = node => node.expandedSubtreeSize;
};
