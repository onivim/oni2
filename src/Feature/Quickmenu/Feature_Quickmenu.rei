open Oni_Core;

// SCHEMA

module Schema: {
  type menu('outmsg);

  module Renderer: {
    type t('item) =
      (
        ~theme: ColorTheme.Colors.t,
        ~font: UiFont.t,
        ~text: string,
        ~highlights: list((int, int)),
        'item
      ) =>
      Revery.UI.element;

    let default: t(_);

    let defaultWithIcon:
      ('item => option(IconTheme.IconDefinition.t)) => t('item);
  };

  let menu:
    (
      ~onItemFocused: 'item => 'outmsg=?,
      ~onItemSelected: 'item => 'outmsg=?,
      ~onCancelled: unit => 'outmsg=?,
      ~placeholderText: string=?,
      ~itemRenderer: Renderer.t('item)=?,
      ~toString: 'item => string,
      list('item)
    ) =>
    menu('outmsg);

  let map: ('outmsg => 'b, menu('outmsg)) => menu('b);
};

// MODEL

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let pasted: string => msg;
};

type model('outmsg);

let initial: model(_);

let show: (~menu: Schema.menu('outmsg), model('outmsg)) => model('outmsg);

let next: model('outmsg) => (model('outmsg), Isolinear.Effect.t('outmsg));
let prev: model('outmsg) => (model('outmsg), Isolinear.Effect.t('outmsg));

let cancel: model('outmsg) => model('outmsg);

let select:
  model('outmsg) => (model('outmsg), Isolinear.Effect.t('outmsg));

let isMenuOpen: model(_) => bool;

// UPDATE

type outmsg('action) =
  | Action('action)
  | Nothing;

let update: (msg, model('action)) => (model('action), outmsg('action));

// SUBSCRIPTION

let sub: model('action) => Isolinear.Sub.t(msg);

// VIEW
module View: {
  let make:
    (
      ~font: UiFont.t,
      ~theme: ColorTheme.Colors.t,
      ~config: Config.resolver,
      ~model: model(_),
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
  // CONTRIBUTIONS
};
module Contributions: {let contextKeys: model(_) => WhenExpr.ContextKeys.t;};
