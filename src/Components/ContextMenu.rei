open Oni_Core;

open Revery.UI;

type identity;

type item('data) = {
  label: string,
  // icon: option(IconTheme.IconDefinition.t),
  data: 'data,
};

type t('data);

let create:
  (
    ~originX: [ | `Left | `Middle | `Right]=?,
    ~originY: [ | `Top | `Middle | `Bottom]=?,
    identity,
    list(item('data))
  ) =>
  t('data);

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

module Identity: {
  let make:
    (~children: identity => React.element(React.node), unit) =>
    React.element(React.node);
};

module Anchor: {
  let make:
    (
      ~identity: identity,
      ~model: option(t('data)),
      ~onUpdate: t('data) => unit,
      unit
    ) =>
    React.element(React.node);
};
