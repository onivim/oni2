/*
 * Tab.re
 *
 */

open Revery;
open Revery.UI;

open Oni_Core;

module Model = Oni_Model;
module Ext = Oni_Extensions;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;

module Constants = {
  include Constants;

  let minWidth = 125;
};

let proportion = p => float(Constants.minWidth) *. p |> int_of_float;

module Styles = {
  open Style;

  let horizontalBorder = (tabPosition, numberOfTabs) =>
    Style.(
      switch (tabPosition, numberOfTabs) {
      /* A single tab should have no borders */
      | (1, 1) => []
      /* The last tab should also have no borders */
      | (i, l) when i == l => []
      /* Every other tab should have a right border */
      | (_, _) => [
          borderRight(~width=1, ~color=Color.rgba(0., 0., 0., 0.1)),
        ]
      }
    );

  let container =
      (
        ~mode,
        ~isActive,
        ~showHighlight,
        ~tabPosition,
        ~numberOfTabs,
        ~theme: Theme.t,
      ) => {
    let (modeColor, _) = Theme.getColorsForMode(theme, mode);

    let borderColor =
      isActive && showHighlight ? modeColor : Colors.transparentBlack;

    [
      overflow(`Hidden),
      paddingHorizontal(5),
      backgroundColor(
        isActive ? theme.tabActiveBackground : theme.tabInactiveBackground,
      ),
      borderTop(~color=borderColor, ~width=2),
      borderBottom(~color=theme.editorBackground, ~width=2),
      height(Constants.tabHeight),
      minWidth(Constants.minWidth),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      ...horizontalBorder(tabPosition, numberOfTabs),
    ];
  };

  let text = (~isActive, ~showHighlight, ~uiFont: UiFont.t, ~theme: Theme.t) => [
    width(proportion(0.80) - 10),
    textOverflow(`Ellipsis),
    fontFamily(
      isActive && showHighlight ? uiFont.fontFileItalic : uiFont.fontFile,
    ),
    fontSize(uiFont.fontSize),
    color(isActive ? theme.tabActiveForeground : theme.tabInactiveForeground),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let icon = [
    width(32),
    height(Constants.tabHeight),
    alignItems(`Center),
    justifyContent(`Center),
  ];
};

let make =
    (
      ~title,
      ~tabPosition,
      ~numberOfTabs,
      ~isActive,
      ~modified,
      ~onClick,
      ~onClose,
      ~theme,
      ~uiFont: UiFont.t,
      ~mode,
      ~showHighlight: bool,
      (),
    ) => {
  let state = GlobalContext.current().state;
  let language =
    Ext.LanguageInfo.getLanguageFromFilePath(state.languageInfo, title);
  let fileIcon: option(Model.IconTheme.IconDefinition.t) =
    Model.IconTheme.getIconForFile(state.iconTheme, title, language);

  let fileIconView =
    switch (fileIcon) {
    | Some(icon) =>
      <FontIcon
        fontFamily="seti.ttf"
        icon={icon.fontCharacter}
        color={icon.fontColor}
        /* TODO: Use 'weight' value from IconTheme font */
        fontSize={uiFont.fontSize *. 1.5}
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

  <View
    style={Styles.container(
      ~mode,
      ~isActive,
      ~showHighlight,
      ~tabPosition,
      ~numberOfTabs,
      ~theme,
    )}>
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
      <View style=Styles.icon> fileIconView </View>
      <Text
        style={Styles.text(~isActive, ~showHighlight, ~uiFont, ~theme)}
        text=title
      />
    </Sneakable>
    <Sneakable onClick=onClose style=Styles.icon>
      <FontIcon
        icon={modified ? FontAwesome.circle : FontAwesome.times}
        color={theme.tabActiveForeground}
        fontSize={modified ? 10. : 12.}
      />
    </Sneakable>
  </View>;
};
