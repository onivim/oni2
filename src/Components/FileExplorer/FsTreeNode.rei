open Oni_Core;

[@deriving show({with_path: false})]
type metadata = {
  path: FpExp.t(FpExp.absolute),
  displayName: string,
  hash: int // hash of basename, so only comparable locally
};

[@deriving show({with_path: false})]
type t = Tree.t(metadata, metadata);

let file: FpExp.t(FpExp.absolute) => t;
let directory:
  (~isOpen: bool=?, FpExp.t(FpExp.absolute), ~children: list(t)) => t;

let getPath: t => FpExp.t(FpExp.absolute);
let displayName: t => string;

let findNodesByPath:
  (FpExp.t(FpExp.absolute), t) =>
  [ | `Success(list(t)) | `Partial(t) | `Failed];
let findByPath: (FpExp.t(FpExp.absolute), t) => option(t);

let replace: (~replacement: t, t) => t;
let updateNodesInPath: (t => t, FpExp.t(FpExp.absolute), t) => t;
let toggleOpen: t => t;
let setOpen: t => t;

let equals: (t, t) => bool;

module Model: {
  type nonrec t = t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
};
