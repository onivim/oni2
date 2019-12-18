open Oni_Core;

module ArrayEx = Utility.ArrayEx;

type t = {
  id: int,
  path: string,
  displayName: string,
  hash: int, // hash of basename, so only comparable locally
  icon: option(IconTheme.IconDefinition.t),
  kind,
  expandedSubtreeSize: int,
}

and kind =
  | Directory({
      isOpen: bool,
      children: list(t),
    })
  | File;

let rec countExpandedSubtree =
  fun
  | Directory({isOpen: true, children}) =>
    List.fold_left(
      (acc, child) => acc + countExpandedSubtree(child.kind),
      1,
      children,
    )

  | _ => 1;

let file = (path, ~id, ~icon) => {
  let basename = Filename.basename(path);

  {
    id,
    path,
    displayName: basename,
    hash: Hashtbl.hash(basename),
    icon,
    kind: File,
    expandedSubtreeSize: 1,
  };
};

let directory = (~isOpen=false, path, ~id, ~icon, ~children) => {
  let kind = Directory({isOpen, children});
  let basename = Filename.basename(path);

  {
    id,
    path,
    displayName: basename,
    hash: Hashtbl.hash(basename),
    icon,
    kind,
    expandedSubtreeSize: countExpandedSubtree(kind),
  };
};

let findNodesByPath = (path, tree) => {
  let path = Workspace.toRelativePath(tree.path, path);
  let pathHashes =
    path
    |> String.split_on_char(Filename.dir_sep.[0])
    |> List.map(Hashtbl.hash);

  let rec loop = (focusedNodes, children, pathSegments) =>
    switch (pathSegments) {
    | [] => `Success(focusedNodes |> List.rev)
    | [hash, ...rest] =>
      switch (children) {
      | [] =>
        let last = focusedNodes |> List.hd;
        last.id == tree.id ? `Failed : `Partial(last);

      | [node, ...children] =>
        if (node.hash == hash) {
          let children =
            switch (node.kind) {
            | Directory({children, _}) => children
            | File => []
            };
          loop([node, ...focusedNodes], children, rest);
        } else {
          loop(focusedNodes, children, pathSegments);
        }
      }
    };

  switch (tree.kind) {
  | Directory({children, _}) => loop([tree], children, pathHashes)
  | File => `Failed
  };
};

let findByPath = (path, tree) => {
  let path = Workspace.toRelativePath(tree.path, path);
  let pathHashes =
    path
    |> String.split_on_char(Filename.dir_sep.[0])
    |> List.map(Hashtbl.hash);

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
  | Directory({children, _}) => loop(tree, children, pathHashes)
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
      let index = children |> ArrayEx.findIndex(child => child.id == focus.id);

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
        let index =
          children |> ArrayEx.findIndex(child => child.id == focus.id);

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

let update = (~tree, ~updater, nodeId) => {
  let rec loop =
    fun
    | {id, _} as node when id == nodeId => updater(node)

    | {kind: Directory({children, _} as dir), _} as node => {
        let kind = Directory({...dir, children: List.map(loop, children)});
        {...node, kind, expandedSubtreeSize: countExpandedSubtree(kind)};
      }

    | node => node;

  loop(tree);
};

let updateNodesInPath = (~tree, ~updater, nodes) => {
  let rec loop = (nodes, node) =>
    switch (nodes) {
    | [{id, kind, _}, ...rest] when id == node.id =>
      switch (kind) {
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

  loop(nodes, tree);
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
