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

type nodeOrLeaf('node, 'leaf) =
| Node({ expanded: bool, indentation: int, data: 'node })
| Leaf({ indentation: int, data: 'leaf });

// UPDATE

type outmsg('node, 'leaf) =
  | Nothing
  | Expanded('node)
  | Collapsed('node)
  | Selected('leaf);


let update: (msg, model('node, 'leaf)) => (model('node, 'leaf), outmsg('node, 'leaf));

let set: (list(Tree.t('node, 'leaf)), model('node, 'leaf)) => model('node, 'leaf);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(unit));
};

// VIEW

module View: {
  let make:
    (
      ~theme: ColorTheme.Colors.t,
      ~model: model('node, 'leaf),
      ~dispatch: msg => unit,
      ~render: (
                 ~availableWidth: int,
                 ~index: int,
                 ~hovered: bool,
                 ~focused: bool,
                 nodeOrLeaf('node, 'leaf)
               ) =>
               Revery.UI.element,
      unit
    ) =>
    Revery.UI.element;
};
