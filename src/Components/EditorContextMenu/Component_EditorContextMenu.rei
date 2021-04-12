open EditorCoreTypes;
open Oni_Core;

module Schema: {
  type t('item);

  module Renderer: {
    type t('item) =
      (~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, 'item) =>
      Revery.UI.element;

    let default: (~toString: 'item => string) => t('item);
  };

  let contextMenu: (~renderer: Renderer.t('item)) => t('item);
};

type model('item);

let create: (~schema: Schema.t('item), list('item)) => model('item);

let set: (~items: list('item), model('item)) => model('item);

type msg('item);

type outmsg('item) =
  | Nothing
  | Cancelled
  | FocusChanged('item)
  | Selected('item);

let configurationChanged:
  (~config: Config.resolver, model('item)) => model('item);

let update: (msg('item), model('item)) => (model('item), outmsg('item));

let sub:
  (
    ~isVisible: bool,
    ~pixelPosition: option(PixelPosition.t),
    model('item)
  ) =>
  Isolinear.Sub.t(msg('item));

module View: {
  let make:
    (
      ~theme: ColorTheme.Colors.t,
      ~uiFont: UiFont.t,
      ~model: model(_),
      unit
    ) =>
    Revery.UI.element;
};
