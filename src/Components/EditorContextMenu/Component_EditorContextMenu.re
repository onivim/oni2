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
      <Revery.UI.View
        style=Revery.UI.Style.[
          position(`Absolute),
          backgroundColor(bg),
          borderRadius(4.),
          top(0),
          left(0),
          right(0),
          bottom(0),
          flexDirection(`Row),
          alignItems(`Center),
        ]>
        <Revery.UI.Text
          fontFamily={uiFont.family}
          fontSize={uiFont.size}
          text={toString(item)}
        />
      </Revery.UI.View>;
    };
  };

  type t('item) = {renderer: Renderer.t('item)};

  let contextMenu = (~renderer) => {renderer: renderer};
};

module Constants = {
  let maxWidth = 500.;
  let maxHeight = 500.;
};

type model('item) = {
  schema: Schema.t('item),
  popup: Component_Popup.model,
  items: array('item),
  selected: option(int),
};

let create = (~schema, items) => {
  schema,
  popup:
    Component_Popup.create(
      ~width=Constants.maxWidth,
      ~height=Constants.maxHeight,
    ),
  items: Array.of_list(items),
  selected: None,
};

let set = (~items, model) => {...model, items: Array.of_list(items)};

type command =
  | AcceptSelected
  | SelectPrevious
  | SelectNext;

type msg('item) =
  | Command(command)
  | Popup(Component_Popup.msg)
  | MouseOver(int);

type outmsg('item) =
  | Nothing
  | Cancelled
  | FocusChanged('item)
  | Selected('item);

let configurationChanged = (~config, model) => {
  ...model,
  popup: Component_Popup.configurationChanged(~config, model.popup),
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

let update = (msg: msg('item), model) => {
  switch (msg) {
  | Command(SelectNext) =>
    let model' = Internal.selectNext(model);
    let eff =
      switch (Internal.selected(model)) {
      | None => Nothing
      | Some(item) => FocusChanged(item)
      };
    (model', eff);
  | Command(SelectPrevious) =>
    let model' = Internal.selectPrevious(model);
    let eff =
      switch (Internal.selected(model)) {
      | None => Nothing
      | Some(item) => FocusChanged(item)
      };
    (model', eff);
  | Command(AcceptSelected) =>
    let eff =
      switch (Internal.selected(model)) {
      | None => Cancelled
      | Some(item) => Selected(item)
      };

    (model, eff);

  | MouseOver(index) =>
    let model' =
      {...model, selected: Some(index)} |> Internal.ensureSelectionInRange;

    let eff =
      switch (Internal.selected(model)) {
      | None => Nothing
      | Some(item) => FocusChanged(item)
      };
    (model', eff);

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
  let make = (~theme, ~uiFont, ~dispatch, ~model, ()) => {
    let bg = Feature_Theme.Colors.EditorSuggestWidget.background.from(theme);
    let fg = Feature_Theme.Colors.EditorSuggestWidget.foreground.from(theme);

    <Component_Popup.View
      model={model.popup}
      inner={(~transition as _) => {
        <View
          style=Style.[
            position(`Absolute),
            backgroundColor(bg),
            color(fg),
            width(500),
            height(100),
            borderRadius(8.),
            boxShadow(
              ~xOffset=4.,
              ~yOffset=4.,
              ~blurRadius=12.,
              ~spreadRadius=0.,
              ~color=Revery.Color.rgba(0., 0., 0., 0.75),
            ),
            pointerEvents(`Allow),
          ]>
          <View
            style=Style.[
              position(`Absolute),
              top(4),
              left(4),
              right(4),
              bottom(4),
            ]>
            <Oni_Components.FlatList
              rowHeight=20
              initialRowsToRender=5
              count={Array.length(model.items)}
              theme
              focused={model.selected}>
              ...{index => {
                let isFocused = model.selected == Some(index);
                let item = model.items[index];
                <Revery.UI.View
                  onMouseOver={item => dispatch(MouseOver(index))}
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
      }}
    />;
  };
};

module KeyBindings = {
  open Feature_Input.Schema;

  let contextMenuVisible = "contextMenuVisible" |> WhenExpr.parse;

  let nextSuggestion =
    bind(
      ~key="<C-N>",
      ~command=Commands.selectNextContextItem.id,
      ~condition=contextMenuVisible,
    );

  let nextSuggestionArrow =
    bind(
      ~key="<DOWN>",
      ~command=Commands.selectNextContextItem.id,
      ~condition=contextMenuVisible,
    );

  let previousSuggestion =
    bind(
      ~key="<C-P>",
      ~command=Commands.selectPrevContextItem.id,
      ~condition=contextMenuVisible,
    );
  let previousSuggestionArrow =
    bind(
      ~key="<UP>",
      ~command=Commands.selectPrevContextItem.id,
      ~condition=contextMenuVisible,
    );

  let acceptSuggestionEnter =
    bind(
      ~key="<CR>",
      ~command=Commands.acceptContextItem.id,
      ~condition=contextMenuVisible,
    );

  let acceptSuggestionTab =
    bind(
      ~key="<TAB>",
      ~command=Commands.acceptContextItem.id,
      ~condition=contextMenuVisible,
    );
};

module Contributions = {
  let commands = _model => {
    Commands.[
      acceptContextItem,
      selectNextContextItem,
      selectPrevContextItem,
    ];
  };

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

  let keybindings =
    KeyBindings.[
      previousSuggestion,
      previousSuggestionArrow,
      nextSuggestionArrow,
      nextSuggestion,
      acceptSuggestionEnter,
      acceptSuggestionTab,
    ];
};
