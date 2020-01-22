open Oni_Core;

open Revery.UI;

[@deriving show]
type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
};

type t('data);

module Overlay: {
  let make:
    (
      ~model: t('data),
      ~theme: Theme.t,
      ~font: UiFont.t,
      ~onOverlayClick: unit => unit,
      ~onItemSelect: item('data) => unit,
      unit
    ) =>
    React.element(React.node);
};

module Make:
  () =>
   {
    let init: list(item('data)) => t('data);

    module Anchor: {
      let make:
        (
          ~model: option(t('data)),
          ~orientation: (
                          [ | `Top | `Middle | `Bottom],
                          [ | `Left | `Middle | `Right],
                        )
                          =?,
          ~offsetX: int=?,
          ~offsetY: int=?,
          ~onUpdate: t('data) => unit,
          unit
        ) =>
        React.element(React.node);
    };
  };
