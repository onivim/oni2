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

let create: (~rowHeight: int, list('item)) => model('item);

type outmsg =
  | Nothing;

// UPDATE

let update: (msg, model('item)) => (model('item), outmsg);

let set: (list('item), model('item)) => model('item);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(unit));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

// VIEW

module View: {

  let make: (
    ~items: array('item),
    ~uniqueId: 'item => 'key,
    ~focused: 'key,
    //~searchText: option('item => string),
    ~scrollY: float,
    ~dispatch: msg => unit,
    ~rowHeight: float,
    ~render: (~width: int=?, 'item) => Revery.UI.element,
    unit,
  ) => Revery.UI.element;
}
