open Oni_Core;
open Revery.UI;

module Overlay: {
  let make:
    (
      ~key: React.Key.t=?,
      ~theme: ColorTheme.resolver,
      ~font: UiFont.t,
      unit
    ) =>
    element;
};

let make:
  (
    ~children: element,
    ~text: string,
    ~style: list(Style.viewStyleProps)=?,
    unit
  ) =>
  element;
