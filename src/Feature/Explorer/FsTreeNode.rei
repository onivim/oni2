open Oni_Core;

[@deriving show({with_path: false})]
type metadata = {
  path: string,
  displayName: string,
  hash: int // hash of basename, so only comparable locally
};

[@deriving show({with_path: false})]
type t = Tree.t(metadata, metadata);

let file: string => t;
let directory: (~isOpen: bool=?, string, ~children: list(t)) => t;

let getPath: t => string;
let displayName: t => string;

let findNodesByPath:
  (string, t) => [ | `Success(list(t)) | `Partial(t) | `Failed];
let findByPath: (string, t) => option(t);

let replace: (~replacement: t, t) => t;
let updateNodesInPath: (t => t, string, t) => t;
let toggleOpen: t => t;
let setOpen: t => t;

let equals: (t, t) => bool;

module Model: {
  type nonrec t = t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
};
