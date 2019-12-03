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
        children: [ | `Loading | `Loaded(list(t))],
      })
    | File;

let file: (string, ~id: int, ~icon: option(IconTheme.IconDefinition.t)) => t;
let directory:
  (
    ~isOpen: bool=?,
    string,
    ~id: int,
    ~icon: option(IconTheme.IconDefinition.t),
    ~children: [ | `Loading | `Loaded(list(t))]
  ) =>
  t;

let update: (~tree: t, ~updater: t => t, int) => t;
let toggleOpenState: t => t;

module Model: {
  type nonrec t = t;

  let children: t => [ | `Loading | `Loaded(list(t))];
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};
