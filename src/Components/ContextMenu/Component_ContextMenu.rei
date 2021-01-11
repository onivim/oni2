open Oni_Core;
open Revery.UI;

[@deriving show]
type item('data) = {
  label: string,
  // TODO: icon: option(IconTheme.IconDefinition.t),
  data: [@opaque] 'data,
  details: [@opaque] element,
};

type content('data) =
  | Item(item('data))
  | Group(list(content('data)))
  | Submenu({
      label: string,
      items: list(content('data)),
    });

// MODEL

type model('data);

let map: (~f: item('data) => item('data), model('data)) => model('data);

[@deriving show]
type msg('data);

let make: list(content('data)) => model('data);

type outmsg('data) =
  | Nothing
  | Selected({data: 'data})
  | Cancelled;

let update: (msg('data), model('data)) => (model('data), outmsg('data));

// VIEW

module View: {
  module Overlay: {let make: (~key: React.Key.t=?, unit) => element;};
  let make:
    //~items: list(item('data)),
    (
      ~model: model('data),
      ~orientation: (
                      [ | `Top | `Middle | `Bottom],
                      [ | `Left | `Middle | `Right],
                    )
                      =?,
      ~offsetX: int=?,
      ~offsetY: int=?,
      ~dispatch: msg('data) => unit,
      //~onItemSelect: item('data) => unit,
      //~onCancel: unit => unit,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      unit
    ) =>
    element;
};
