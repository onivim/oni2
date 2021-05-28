open Oni_Core;

module Schema = {
  module Renderer = {
    type t('item) =
      (
        ~isFocused: bool,
        ~theme: ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        'item
      ) =>
      Revery.UI.element;

    let default = (~toString, ~isFocused, ~theme, ~uiFont: UiFont.t, item) => {
      let bg =
        isFocused
          ? Feature_Theme.Colors.EditorSuggestWidget.selectedBackground.from(
              theme,
            )
          : Revery.Colors.transparentBlack;
      let fg =
        Feature_Theme.Colors.EditorSuggestWidget.foreground.from(theme);

      <Revery.UI.View
        style=Revery.UI.Style.[
          position(`Absolute),
          backgroundColor(bg),
          // borderRadius(4.),
          top(0),
          left(0),
          right(0),
          bottom(0),
          flexDirection(`Row),
          alignItems(`Center),
        ]>
        <Revery.UI.View style=Revery.UI.Style.[marginLeft(8)]>
          <Revery.UI.Text
            fontFamily={uiFont.family}
            fontSize={uiFont.size}
            text={toString(item)}
            style=Revery.UI.Style.[color(fg)]
          />
        </Revery.UI.View>
      </Revery.UI.View>;
    };
  };

  type t('item) = {renderer: Renderer.t('item)};

  let contextMenu = (~renderer) => {renderer: renderer};
};

type model('item) = {
  schema: Schema.t('item),
  items: array('item),
  selected: option(int),
};

let create = (~schema, items) => {
  let items = Array.of_list(items);
  let selected = Array.length(items) > 0 ? Some(0) : None;
  {schema, items, selected};
};

let set = (~items, model) => {...model, items: Array.of_list(items)};

type msg('item) =
  | MouseOver(int)
  | MouseUp(int)
  | MouseClickedBackground;

type outmsg('item) =
  | Nothing
  | Cancelled
  | Selected('item);

let configurationChanged = (~config as _, model) => {
  model;
};

module Internal = {
  let ensureSelectionInRange = ({items, selected, _} as model) => {
    let len = Array.length(items);
    if (len == 0) {
      {...model, selected: None};
    } else {
      let selected' =
        selected
        |> Option.map(selectedIdx =>
             if (selectedIdx >= len) {
               0;
             } else if (selectedIdx < 0) {
               len - 1;
             } else {
               selectedIdx;
             }
           );
      {...model, selected: selected'};
    };
  };

  let selectNext = ({selected, _} as model) => {
    let selected' =
      switch (selected) {
      | None => Some(0)
      | Some(idx) => Some(idx + 1)
      };

    {...model, selected: selected'} |> ensureSelectionInRange;
  };

  let selectPrevious = ({selected, _} as model) => {
    let selected' =
      switch (selected) {
      | None => Some(0)
      | Some(idx) => Some(idx + 1)
      };

    {...model, selected: selected'} |> ensureSelectionInRange;
  };

  let selected = model => {
    let model' = model |> ensureSelectionInRange;

    model'.selected |> Option.map(idx => {model'.items[idx]});
  };
};

let next = model => model |> Internal.selectNext;
let previous = model => model |> Internal.selectPrevious;
let selected = model => model |> Internal.selected;

let update = (msg: msg('item), model) => {
  switch (msg) {
  | MouseOver(index) =>
    let model' =
      {...model, selected: Some(index)} |> Internal.ensureSelectionInRange;

    (model', Nothing);

  | MouseUp(_index) =>
    let eff =
      switch (Internal.selected(model)) {
      | None => Nothing
      | Some(item) => Selected(item)
      };

    (model, eff);

  | MouseClickedBackground => (model, Cancelled)
  };
};

let sub = _model => {
  Isolinear.Sub.none;
};

module View = {
  open Revery.UI;
  let make =
      (
        ~pixelPosition: EditorCoreTypes.PixelPosition.t,
        ~theme,
        ~uiFont,
        ~dispatch,
        ~model,
        (),
      ) => {
    let bg = Feature_Theme.Colors.EditorSuggestWidget.background.from(theme);
    let fg = Feature_Theme.Colors.EditorSuggestWidget.foreground.from(theme);

    let count = Array.length(model.items);
    let marginSize = 5;
    let doubleMarginSize = marginSize * 2;
    let rowHeight = 24;
    let rowsToRender = 5;
    let pixelHeight =
      min(
        count * rowHeight + doubleMarginSize,
        rowsToRender * rowHeight + doubleMarginSize,
      );

    <View
      onMouseUp={_ => dispatch(MouseClickedBackground)}
      style=Style.[
        position(`Absolute),
        top(0),
        left(0),
        right(0),
        bottom(0),
        backgroundColor(
          Revery.Colors.black |> Revery.Color.multiplyAlpha(0.1),
        ),
        pointerEvents(`Allow),
      ]>
      <View
        style=Style.[
          position(`Absolute),
          backgroundColor(bg),
          color(fg),
          left(int_of_float(pixelPosition.x)),
          top(int_of_float(pixelPosition.y)),
          width(500),
          height(pixelHeight),
          //borderRadius(8.),
          boxShadow(
            ~xOffset=4.,
            ~yOffset=4.,
            ~blurRadius=12.,
            ~spreadRadius=0.,
            ~color=Revery.Color.rgba(0., 0., 0., 0.75),
          ),
        ]>
        <View
          style=Style.[
            position(`Absolute),
            top(marginSize),
            left(marginSize),
            right(marginSize),
            bottom(marginSize),
          ]>
          <Oni_Components.FlatList
            rowHeight
            initialRowsToRender=rowsToRender
            count
            theme
            focused={model.selected}>
            ...{index => {
              let isFocused = model.selected == Some(index);
              let item = model.items[index];
              <Revery.UI.View
                onMouseOver={_item => dispatch(MouseOver(index))}
                onMouseUp={_item => dispatch(MouseUp(index))}
                style=Revery.UI.Style.[
                  position(`Absolute),
                  backgroundColor(bg),
                  top(0),
                  left(0),
                  right(0),
                  bottom(0),
                ]>
                {model.schema.renderer(~isFocused, ~theme, ~uiFont, item)}
              </Revery.UI.View>;
            }}
          </Oni_Components.FlatList>
        </View>
      </View>
    </View>;
  };
};
