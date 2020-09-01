/*
     Tree.re

     Stubs for bindings to the `TSTree` object
 */

type t;

external _getRootNode: t => NativeTypes.node = "rets_tree_root_node";

external edit: (t, int, int, int, int, int, int) => t =
  "rets_tree_edit_bytecode" "rets_tree_edit_native";

let getRootNode = (v: t) => {
  let node = _getRootNode(v);
  (v, node);
};
