open EditorCoreTypes;
open Oni_Core;

module Constants = {
  let maxWidth = 500.;
  let maxHeight = 500.;
  let rowHeight = 20;
};

type model('item) = {
  popup: Component_Popup.model,
  items: list('item),
};

let create = items => {
  popup:
    Component_Popup.create(
      ~width=Constants.maxWidth,
      ~height=Constants.maxHeight,
    ),
  items,
};

let initial = [] |> create;

let set = (~items, model) => {...model, items};

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
  let make = (~model, ()) => {
    <Component_Popup.View
      model={model.popup}
      inner={(~transition) => {<Text text="Hello world" />}}
    />;
  };
};
