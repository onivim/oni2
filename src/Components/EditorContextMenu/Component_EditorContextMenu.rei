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
  | FocusChanged('item)
  | Selected('item);

let update: (msg('item), model('item)) => (model('item), outmsg('item));

module Contributions: {
  let commands: model('item) => list(Oni_Core.Command.t(msg('item)));

  let contextKeys: model(_) => WhenExpr.ContextKeys.t;

  let keybindings: list(Feature_Input.Schema.keybinding);
};

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
      ~dispatch: msg('item) => unit,
      ~model: model('item),
      unit
    ) =>
    Revery.UI.element;
};
