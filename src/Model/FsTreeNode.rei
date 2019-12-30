type t =
  pri {
    path: string,
    displayName: string,
    hash: int, // hash of basename, so only comparable locally
    icon: option(IconTheme.IconDefinition.t),
    kind,
    expandedSubtreeSize: int,
  }

and kind =
  pri
    | Directory({
        isOpen: bool,
        children: list(t),
      })
    | File;

let file: (string, ~icon: option(IconTheme.IconDefinition.t)) => t;
let directory:
  (
    ~isOpen: bool=?,
    string,
    ~icon: option(IconTheme.IconDefinition.t),
    ~children: list(t)
  ) =>
  t;

let findNodesByPath:
  (string, t) => [ | `Success(list(t)) | `Partial(t) | `Failed];
let findByPath: (string, t) => option(t);

let prevExpandedNode: (string, t) => option(t);
let nextExpandedNode: (string, t) => option(t);

let expandedIndex: (string, t) => option(int);

let update: (~tree: t, ~updater: t => t, string) => t;
let updateNodesInPath: (~tree: t, ~updater: t => t, list(t)) => t;
let toggleOpen: t => t;
let setOpen: t => t;

let equals: (t, t) => bool;

module Model: {
  type nonrec t = t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};
