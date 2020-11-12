open Oni_Core;
open Revery.UI;

module Overlay: {
  let make:
    (
      ~key: React.Key.t=?,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      unit
    ) =>
    element;
};

let make:
  (
    ~offsetX: int=?,
    ~offsetY: int=?,
    ~children: element,
    ~text: string,
    ~style: list(Style.viewStyleProps)=?,
    unit
  ) =>
  element;
