open EditorCoreTypes;
open Oni_Core;

type model('item);

module Schema: {
  type t('item);

  module Renderer: {
    type t('item) =
      (
        ~isFocused: bool,
        ~theme: ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        'item
      ) =>
      Revery.UI.element;

    let default: (~toString: 'item => string) => t('item);
  };

  let contextMenu: (~renderer: Renderer.t('item)) => t('item);
};

let create: (~schema: Schema.t('item), list('item)) => model('item);

let configurationChanged:
  (~config: Oni_Core.Config.resolver, model('item)) => model('item);

let set: (~items: list('item), model('item)) => model('item);

type msg('item);

type outmsg('item) =
  | Nothing
  | Cancelled
  | Selected('item);

let update: (msg('item), model('item)) => (model('item), outmsg('item));

let next: model('item) => model('item);
let previous: model('item) => model('item);

let selected: model('item) => option('item);

let sub: model('item) => Isolinear.Sub.t(msg('item));

module View: {
  let make:
    (
      ~pixelPosition: PixelPosition.t,
      ~theme: ColorTheme.Colors.t,
      ~uiFont: UiFont.t,
      ~dispatch: msg('item) => unit,
      ~model: model('item),
      unit
    ) =>
    Revery.UI.element;
};
