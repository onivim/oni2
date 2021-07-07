/*
 * Component_VimTree
 *
 * A component that features can use to enable vim-style tree navigation.
 */

open Oni_Core;

// MODEL

[@deriving show]
type msg;

[@deriving show]
type model('node, 'leaf);

let create: (~rowHeight: int) => model('node, 'leaf);

let count: model('node, 'leaf) => int;

type nodeOrLeaf('node, 'leaf) =
  | Node({
      expanded: bool,
      indentation: int,
      data: 'node,
    })
  | Leaf({
      indentation: int,
      data: 'leaf,
    });

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Touched('leaf)
  | Selected('leaf)
  | SelectedNode('node);

let update:
  (msg, model('node, 'leaf)) => (model('node, 'leaf), outmsg('node, 'leaf));

let set:
  (
    ~searchText: nodeOrLeaf('node, 'leaf) => string=?,
    ~uniqueId: 'node => string,
    list(Tree.t('node, 'leaf)),
    model('node, 'leaf)
  ) =>
  model('node, 'leaf);

let keyPress: (string, model('node, 'leaf)) => model('node, 'leaf);

let findIndex:
  (nodeOrLeaf('node, 'leaf) => bool, model('node, 'leaf)) => option(int);

let setSelected:
  (~selected: int, model('node, 'leaf)) => model('node, 'leaf);

let selected: model('node, 'leaf) => option(nodeOrLeaf('node, 'leaf));

let selectNextNode: model('node, 'leaf) => model('node, 'leaf);

let selectPreviousNode: model('node, 'leaf) => model('node, 'leaf);

let scrollTo:
  (
    ~index: int,
    ~alignment: [< | `Top | `Bottom | `Center | `Reveal],
    model('node, 'leaf)
  ) =>
  model('node, 'leaf);

let collapse: model('node, 'leaf) => model('node, 'leaf);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let keybindings: list(Feature_Input.Schema.keybinding);
  let contextKeys: model('node, 'leaf) => WhenExpr.ContextKeys.t;
};

// VIEW

module View: {
  let make:
    (
      ~config: Config.resolver,
      ~isActive: bool,
      ~font: UiFont.t,
      ~focusedIndex: option(int),
      ~theme: ColorTheme.Colors.t,
      ~model: model('node, 'leaf),
      ~dispatch: msg => unit,
      ~render: (
                 ~availableWidth: int,
                 ~index: int,
                 ~hovered: bool,
                 ~selected: bool,
                 nodeOrLeaf('node, 'leaf)
               ) =>
               Revery.UI.element,
      unit
    ) =>
    Revery.UI.element;
};
