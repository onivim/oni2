open Revery.Math;
open Oni_Core;

type sneakInfo = {
  callback: unit => unit,
  boundingBox: BoundingBox2d.t,
};

type sneak = {
  callback: unit => unit,
  boundingBox: BoundingBox2d.t,
  id: string,
};

[@deriving show({with_path: false})]
type msg =
  | NoneAvailable
  | Executed([@opaque] sneak)
  | Discovered([@opaque] list(sneakInfo))
  | KeyboardInput(string);

type model;

let initial: model;

let getTextHighlight: (string, model) => (string, string);

let reset: model => model;
let hide: model => model;

let isActive: model => bool;

let refine: (string, model) => model;

let add: (list(sneakInfo), model) => model;

let getFiltered: model => list(sneak);

// REGISTRY

module Registry: {
  let getSneaks: unit => list(sneakInfo);

  let register: (ref(option(Revery.UI.node)), unit => unit) => unit;
  let unregister: ref(option(Revery.UI.node)) => unit;
};

module View: {
  module Sneakable: {
    let make:
      (
        ~key: Brisk_reconciler.Key.t=?,
        ~style: list(Revery_UI.Style.viewStyleProps)=?,
        ~onClick: unit => unit=?,
        ~onRightClick: unit => unit=?,
        ~onAnyClick: Revery_UI.NodeEvents.mouseButtonEventParams => unit=?,
        ~onSneak: unit => unit=?,
        ~onBlur: Revery_UI.NodeEvents.focusHandler=?,
        ~onFocus: Revery_UI.NodeEvents.focusHandler=?,
        ~tabindex: int=?,
        ~onKeyDown: Revery_UI.NodeEvents.keyDownHandler=?,
        ~onKeyUp: Revery_UI.NodeEvents.keyUpHandler=?,
        ~onTextEdit: Revery_UI.NodeEvents.textEditHandler=?,
        ~onTextInput: Revery_UI.NodeEvents.textInputHandler=?,
        ~children: Revery.UI.element,
        unit
      ) =>
      Revery.UI.element;
  };

  module Overlay: {
    let make:
      (~model: model, ~theme: ColorTheme.Colors.t, ~font: UiFont.t, unit) =>
      Revery.UI.element;
  };
};
