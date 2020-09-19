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

let count: model('item) => int;

type outmsg =
  | Nothing
  | Selected({index: int});

// UPDATE

let update: (msg, model('item)) => (model('item), outmsg);

let set: (array('item), model('item)) => model('item);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(unit));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

// VIEW

module View: {
  let make:
    //      ~key: Brisk_reconciler.Key.t=?,
    (
      ~theme: ColorTheme.Colors.t,
      ~model: model('item),
      //    ~uniqueId: 'item => 'key,
      //    ~focused: 'key,
      //~searchText: option('item => string),
      //    ~scrollY: float,
      ~dispatch: msg => unit,
      //    ~rowHeight: float,
      //      ~render: 'item => Revery.UI.element,
      ~render: (
                 ~availableWidth: int,
                 ~index: int,
                 ~hovered: bool,
                 ~focused: bool,
                 'item
               ) =>
               Revery.UI.element,
      unit
    ) =>
    Revery.UI.element;
};
