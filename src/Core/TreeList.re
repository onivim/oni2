[@deriving show]
type t('node, 'leaf) =
  | ViewLeaf({
      indentationLevel: int,
      data: 'leaf,
    })
  | ViewNode({
      expanded: bool,
      indentationLevel: int,
      data: 'node,
    });

let isLeaf =
  fun
  | ViewLeaf(_) => true
  | ViewNode(_) => false;

let ofTree: Tree.t('node, 'leaf') => list(t('node, 'leaf)) =
  tree => {
    let rec loop = (acc, node, indentationLevel) => {
      switch (node) {
      | Tree.Leaf(data) => [ViewLeaf({indentationLevel, data}), ...acc]
      | Tree.Node({expanded, data, children}) =>
        let currentRoot = ViewNode({expanded, data, indentationLevel});
        let currentAcc = [currentRoot, ...acc];
        if (expanded) {
          List.fold_left(
            (prev, curr) => {loop(prev, curr, indentationLevel + 1)},
            currentAcc,
            children,
          );
        } else {
          currentAcc;
        };
      };
    };

    loop([], tree, 0) |> List.rev;
  };

let%test_module "ofTree" =
  (module
   {
     module View = {
       let leaf = (~indentation=0, v) =>
         ViewLeaf({indentationLevel: indentation, data: v});
       let node = (~expanded=true, ~indentation=0, v) =>
         ViewNode({expanded, indentationLevel: indentation, data: v});
     };
     let%test "single child" = {
       Tree.leaf(1) |> ofTree == [View.leaf(1)];
     };
     let%test "single parent" = {
       Tree.node(1) |> ofTree == [View.node(1)];
     };
     let%test "parent+child, expanded" = {
       let children = [Tree.leaf(2)];
       let tree = Tree.node(~children, 1) |> ofTree;
       tree == [View.node(1), View.leaf(~indentation=1, 2)];
     };
     let%test "parent+child, not expanded" = {
       let children = [Tree.leaf(2)];
       let tree = Tree.node(~children, ~expanded=false, 1) |> ofTree;
       tree == [View.node(~expanded=false, 1)];
     };
     let%test "parent -> parent -> child" = {
       let children2 = [Tree.leaf(2)];
       let children1 = [Tree.node(~children=children2, ~expanded=true, 1)];
       let tree = Tree.node(~children=children1, ~expanded=true, 0) |> ofTree;
       tree
       == [
            View.node(~expanded=true, 0),
            View.node(~expanded=true, ~indentation=1, 1),
            View.leaf(~indentation=2, 2),
          ];
     };
   });
