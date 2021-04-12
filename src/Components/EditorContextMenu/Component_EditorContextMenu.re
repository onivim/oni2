open EditorCoreTypes;
open Oni_Core;

module Schema = {
  module Renderer = {
    type t('item) =
      (~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, 'item) =>
      Revery.UI.element;

    let default = (~toString, ~theme, ~uiFont: UiFont.t, item) => {
      <Revery.UI.Text fontFamily={uiFont.family} text={toString(item)} />;
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

type command =
  | AcceptSelected
  | SelectPrevious
  | SelectNext;

type msg('item) =
  | Nothing
  | Command(command)
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
  | Command(_) =>
    // TODO
    failwith("TODO");
    prerr_endline("TODO");
    (model, Nothing);
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

module Commands = {
  open Feature_Commands.Schema;
  let acceptContextItem =
    define("acceptContextItem", Command(AcceptSelected));

  let selectPrevContextItem =
    define("selectPrevContextItem", Command(SelectPrevious));

  let selectNextContextItem =
    define("selectNextContextItem", Command(SelectNext));
};

module View = {
  open Revery.UI;
  let make = (~theme, ~uiFont, ~model, ()) => {
    let bg = Feature_Theme.Colors.EditorSuggestWidget.background.from(theme);
    let fg = Feature_Theme.Colors.EditorSuggestWidget.foreground.from(theme);

    <Component_Popup.View
      model={model.popup}
      inner={(~transition) => {
        <View
          style=Style.[
            position(`Absolute),
            backgroundColor(bg),
            color(fg),
            width(500),
            height(100),
            boxShadow(
              ~xOffset=4.,
              ~yOffset=4.,
              ~blurRadius=12.,
              ~spreadRadius=0.,
              ~color=Revery.Color.rgba(0., 0., 0., 0.75),
            ),
          ]>
          //borderRadius(8.),

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
          </View>
      }}
    />;
  };
};

module Contributions = {
  let commands =
    Commands.[
      acceptContextItem,
      selectNextContextItem,
      selectPrevContextItem,
    ];

  let contextKeys = model =>
    WhenExpr.ContextKeys.(
      [
        Schema.bool("contextMenuVisible", ({popup, _}) => {
          Component_Popup.isVisible(popup)
        }),
      ]
      |> Schema.fromList
      |> fromSchema(model)
    );

  let keybindings = [];
};
