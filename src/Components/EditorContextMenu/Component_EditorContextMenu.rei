open EditorCoreTypes;
open Oni_Core;

type model('item);

let create: list('item) => model('item);

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

module View: {let make: (~model: model(_), unit) => Revery.UI.element;};
