/*
 * Component_VimList
 *
 * A component that features can use to enable vim-style list navigation.
 */

open Oni_Core;

// MODEL

[@deriving show]
type msg;

[@deriving show]
type model('item);

let create: (~rowHeight: int) => model('item);

let get: (int, model('item)) => option('item);
let selectedIndex: model('item) => int;

let count: model('item) => int;

type outmsg =
  | Nothing
  | Selected({index: int});

// UPDATE

let update: (msg, model('item)) => (model('item), outmsg);

let set: (array('item), model('item)) => model('item);

let findIndex: ('item => bool, model('item)) => option(int);

let scrollTo:
  (
    ~index: int,
    ~alignment: [< | `Top | `Bottom | `Center | `Reveal],
    model('item)
  ) =>
  model('item);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(unit));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

// VIEW

module View: {
  let make:
    (
      ~isActive: bool,
      ~focusedIndex: option(int),
      ~theme: ColorTheme.Colors.t,
      ~model: model('item),
      ~dispatch: msg => unit,
      ~render: (
                 ~availableWidth: int,
                 ~index: int,
                 ~hovered: bool,
                 ~selected: bool,
                 'item
               ) =>
               Revery.UI.element,
      unit
    ) =>
    Revery.UI.element;
};
