[@deriving show]
type t('node, 'leaf) =
  | Leaf('leaf)
  | Node({
      expanded: bool,
      data: 'node,
      children: list(t('node, 'leaf)),
    });

let leaf = data => Leaf(data);

let node = (~expanded=true, ~children=[], data) => {
  Node({expanded, children, data});
};
