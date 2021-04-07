open EditorCoreTypes;
open Oni_Core;
type model;

let create: (~width: float, ~height: float) => model;

type msg;

let update: (msg, model) => model;

let configurationChanged: (~config: Config.resolver, model) => model;

let sub:
  (~isVisible: bool, ~pixelPosition: option(PixelPosition.t), model) =>
  Isolinear.Sub.t(msg);

module View: {
  let make:
    (~model: model, ~inner: (~transition: float) => Revery.UI.element, unit) =>
    Revery.UI.element;
};
