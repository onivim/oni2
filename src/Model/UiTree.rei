type t =
  pri {
    id: int,
    path: string,
    displayName: string,
    icon: option(Oni_Model__.IconTheme.IconDefinition.t),
    depth: int,
    kind,
    expandedSubtreeSize: int,
  }

and kind =
  | Directory({
      isOpen: bool,
      children: [ | `Loading | `Loaded(list(t))],
    })
  | File;

let create:
  (
    ~id: int,
    ~path: string,
    ~icon: option(Oni_Model__.IconTheme.IconDefinition.t),
    ~depth: int,
    ~kind: kind
  ) =>
  t;

let updateNode: (int, t, ~updater: t => t) => t;

let toggleOpenState: (int, t) => t;

module Model: {
  type nonrec t = t;

  let children: t => [ | `Loading | `Loaded(list(t))];
  let kind: t => [ | `Node([ | `Open | `Closed]) | `Leaf];
  let expandedSubtreeSize: t => int;
};
