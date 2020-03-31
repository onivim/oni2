open Oni_Core;
open Revery.UI;

[@deriving show]
type item('data) = {
  label: string,
  // TODO: icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
};

module Overlay: {
  let make: (~key: React.Key.t=?, ~onClick: unit => unit, unit) => element;
};

let make:
  (
    ~items: list(item('data)),
    ~orientation: (
                    [ | `Top | `Middle | `Bottom],
                    [ | `Left | `Middle | `Right],
                  )
                    =?,
    ~offsetX: int=?,
    ~offsetY: int=?,
    ~onItemSelect: item('data) => unit,
    ~theme: ColorTheme.resolver,
    ~font: UiFont.t,
    unit
  ) =>
  element;
