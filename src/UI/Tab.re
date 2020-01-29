/*
 * Tab.re
 *
 */

open Revery;
open Revery.UI;

open Oni_Core;
module Model = Oni_Model;
module Ext = Oni_Extensions;

type tabAction = unit => unit;

let minWidth_ = 125;
let proportion = p => float_of_int(minWidth_) *. p |> int_of_float;

let horizontalBorderStyles = (tabPosition, numberOfTabs) =>
  Style.(
    switch (tabPosition, numberOfTabs) {
    /* A single tab should have no borders */
    | (1, 1) => []
    /* The last tab should also have no borders */
    | (i, l) when i == l => []
    /* Every other tab should have a right border */
    | (_, _) => [borderRight(~width=1, ~color=Color.rgba(0., 0., 0., 0.1))]
    }
  );

let make =
    (
      ~title,
      ~tabPosition,
      ~numberOfTabs,
      ~active,
      ~modified,
      ~onClick,
      ~onClose,
      ~theme: Theme.t,
      ~uiFont: UiFont.t,
      ~mode: Vim.Mode.t,
      ~showHighlight: bool,
      (),
    ) => {
  let (modeColor, _) = Theme.getColorsForMode(theme, mode);

  let borderColor =
    active && showHighlight ? modeColor : Colors.transparentBlack;

  let containerStyle =
    Style.[
      overflow(`Hidden),
      paddingHorizontal(5),
      backgroundColor(theme.editorBackground),
      borderTop(~color=borderColor, ~width=2),
      borderBottom(~color=theme.editorBackground, ~width=2),
      height(Constants.default.tabHeight),
      minWidth(minWidth_),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      ...horizontalBorderStyles(tabPosition, numberOfTabs),
    ];

  let isBold = active && showHighlight;

  let textStyle =
    Style.[
      width(proportion(0.80) - 10),
      textOverflow(`Ellipsis),
      fontFamily(isBold ? uiFont.fontFileItalic : uiFont.fontFile),
      fontSize(uiFont.fontSize),
      color(theme.tabActiveForeground),
      backgroundColor(theme.editorBackground),
      justifyContent(`Center),
      alignItems(`Center),
    ];

  let iconContainerStyle =
    Style.[
      width(32),
      height(Constants.default.tabHeight),
      alignItems(`Center),
      justifyContent(`Center),
    ];

  let icon = modified ? FontAwesome.circle : FontAwesome.times;

  let state = GlobalContext.current().state;
  let language =
    Ext.LanguageInfo.getLanguageFromFilePath(state.languageInfo, title);
  let fileIcon: option(Model.IconTheme.IconDefinition.t) =
    Model.IconTheme.getIconForFile(state.iconTheme, title, language);

  let fileIconView =
    switch (fileIcon) {
    | Some(v) =>
      <FontIcon
        fontFamily="seti.ttf"
        icon={v.fontCharacter}
        backgroundColor={theme.editorBackground}
        color={v.fontColor}
        /* TODO: Use 'weight' value from IconTheme font */
        fontSize={uiFont.fontSize *. 1.5)}
      />
    | None => React.empty
    };

  let onAnyClick = (evt: NodeEvents.mouseButtonEventParams) => {
    switch (evt.button) {
    | Revery.MouseButton.BUTTON_MIDDLE => onClose()
    | Revery.MouseButton.BUTTON_LEFT => onClick()
    | _ => ()
    };
  };

  <View style=containerStyle>
    <Sneakable
      onSneak=onClick
      onAnyClick
      style=Style.[
        width(proportion(0.80)),
        flexGrow(1),
        flexDirection(`Row),
        alignItems(`Center),
        justifyContent(`Center),
      ]>
      <View style=iconContainerStyle> fileIconView </View>
      <Text style=textStyle text=title />
    </Sneakable>
    <Sneakable onClick=onClose style=iconContainerStyle>
      <FontIcon
        icon
        backgroundColor={theme.editorBackground}
        color={theme.tabActiveForeground}
        fontSize={modified ? 10 : 12}
      />
    </Sneakable>
  </View>;
};
