type t =
  pri {
    id: int,
    path: string,
    displayName: string,
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

let file: (string, ~id: int, ~icon: option(IconTheme.IconDefinition.t)) => t;
let directory:
  (
    ~isOpen: bool=?,
    string,
    ~id: int,
    ~icon: option(IconTheme.IconDefinition.t),
    ~children: list(t)
  ) =>
  t;

let findNodesByLocalPath:
  (string, t) => [ | `Success(list(t)) | `Partial(t) | `Failed];

let update: (~tree: t, ~updater: t => t, int) => t;
let toggleOpenState: t => t;

module Model: {
  type nonrec t = t;

  let children: t => list(t);
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};
