open EditorCoreTypes;
open Oni_Core;

module Schema = {
  module Renderer = {
    type t('item) =
      (~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, 'item) =>
      Revery.UI.element;

    let default = (~toString, ~theme, ~uiFont, item) => {
      <Revery.UI.Text text={toString(item)} />;
    };
  };

  type t('item) = {renderer: Renderer.t('item)};

  let contextMenu = (~renderer) => {renderer: renderer};
};

module Constants = {
  let maxWidth = 500.;
  let maxHeight = 500.;
  let rowHeight = 20;
};

type model('item) = {
  schema: Schema.t('item),
  popup: Component_Popup.model,
  items: array('item),
};

let create = (~schema, items) => {
  schema,
  popup:
    Component_Popup.create(
      ~width=Constants.maxWidth,
      ~height=Constants.maxHeight,
    ),
  items: Array.of_list(items),
};

let set = (~items, model) => {...model, items: Array.of_list(items)};

type msg('item) =
  | Nothing
  | Popup(Component_Popup.msg);

type outmsg('item) =
  | Nothing
  | Cancelled
  | FocusChanged('item)
  | Selected('item);

let configurationChanged = (~config, model) => {
  ...model,
  popup: Component_Popup.configurationChanged(~config, model.popup),
};

let update = (msg: msg('item), model) => {
  switch (msg) {
  | Nothing => (model, Nothing)
  | Popup(msg) => (
      {...model, popup: Component_Popup.update(msg, model.popup)},
      Nothing,
    )
  };
};

let sub = (~isVisible, ~pixelPosition, model) => {
  Component_Popup.sub(~isVisible, ~pixelPosition, model.popup)
  |> Isolinear.Sub.map(msg => Popup(msg));
};

module View = {
  open Revery.UI;
  let make = (~theme, ~uiFont, ~model, ()) => {
    <Component_Popup.View
      model={model.popup}
      inner={(~transition) => {
        <Oni_Components.FlatList
          rowHeight=20
          initialRowsToRender=5
          count={Array.length(model.items)}
          theme
          focused=None>
          ...{index => {
            let item = model.items[index];
            model.schema.renderer(~theme, ~uiFont, item);
          }}
        </Oni_Components.FlatList>
      }}
    />;
  };
};
