/*
     Node.re

     Stubs for bindings to the `TSTree` object
 */

open EditorCoreTypes;
open NativeTypes;

type t = (Tree.t, node);

let getTree = (v: t) => {
  let (_, tree) = v;
  tree;
};

external _toString: node => string = "rets_node_string";

external _getChildCount: node => int = "rets_node_child_count";
external _getChild: (node, int) => node = "rets_node_child";
external _getParent: node => node = "rets_node_parent";

external _getNamedChildCount: node => int = "rets_node_named_child_count";
external _getNamedChild: (node, int) => node = "rets_node_named_child";

external _getBoundedNamedIndex: node => int = "rets_node_bounded_named_index";
external _getNamedIndex: node => int = "rets_node_named_index";
external _getIndex: node => int = "rets_node_index";

external _getDescendantForPointRange: (node, int, int, int, int) => node =
  "rets_node_descendant_for_point_range";

external _getStartByte: node => int = "rets_node_start_byte";
external _getEndByte: node => int = "rets_node_end_byte";

external _getStartPoint: node => Location.t = "rets_node_start_point";
external _getEndPoint: node => Location.t = "rets_node_end_point";

external _hasChanges: node => bool = "rets_node_has_changes";
external _hasError: node => bool = "rets_node_has_error";

external _isMissing: node => bool = "rets_node_is_missing";
external _isNull: node => bool = "rets_node_is_null";
external _isNamed: node => bool = "rets_node_is_named";
external _isError: node => bool = "rets_node_is_error";
external _isExtra: node => bool = "rets_node_is_extra";

external _getSymbol: node => int = "rets_node_symbol";
external _getType: node => string = "rets_node_type";

let wrap0 = (f, v) => {
  let (_, node) = v;
  f(node);
};

let toString: t => string = wrap0(_toString);
let getChildCount: t => int = wrap0(_getChildCount);

let getChild = (v: t, idx) => {
  let (tree, node) = v;
  (tree, _getChild(node, idx));
};

let getParent: t => t =
  (v: t) => {
    let (tree, node) = v;
    (tree, _getParent(node));
  };

let getNamedChildCount: t => int = wrap0(_getNamedChildCount);

let getNamedChild = (v: t, idx) => {
  let (tree, node) = v;
  (tree, _getNamedChild(node, idx));
};

let getBoundedNamedIndex = (v: t) => {
  let (_, node) = v;
  _getBoundedNamedIndex(node);
};

let getNamedIndex = (v: t) => {
  let (_, node) = v;
  _getNamedIndex(node);
};

let getIndex = (v: t) => {
  let (_, node) = v;
  _getIndex(node);
};

let getDescendantForPointRange = (v: t, r0, c0, r1, c1) => {
  let (tree, node) = v;
  (tree, _getDescendantForPointRange(node, r0, c0, r1, c1));
};

let getStartByte: t => int = wrap0(_getStartByte);

let getEndByte: t => int = wrap0(_getEndByte);

let getStartPoint: t => Location.t = wrap0(_getStartPoint);
let getEndPoint: t => Location.t = wrap0(_getEndPoint);

let hasChanges: t => bool = wrap0(_hasChanges);
let hasError: t => bool = wrap0(_hasError);

let isMissing: t => bool = wrap0(_isMissing);
let isNull: t => bool = wrap0(_isNull);
let isNamed: t => bool = wrap0(_isNamed);
let isError: t => bool = wrap0(_isError);
let isExtra: t => bool = wrap0(_isExtra);

let getSymbol: t => int = wrap0(_getSymbol);
let getType: t => string = wrap0(_getType);

let getChildren = (node: t) => {
  let i = ref(0);
  let count = getChildCount(node);

  let children = ref([]);

  while (i^ < count) {
    let child = getChild(node, i^);
    children := [child, ...children^];
    incr(i);
  };

  List.rev(children^);
};

let getRange = (node: t) => {
  let start = getStartPoint(node);
  let stop = getEndPoint(node);

  Range.create(~start, ~stop);
};
