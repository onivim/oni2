/*
 * PaneTab.re
 *
 * A tab in the lower pane
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

type tabAction = unit => unit;

let minWidth_ = 100;

let make = (~uiFont, ~theme, ~title, ~onClick, ~active, ()) => {
  let (modeColor, _) = Theme.getColorsForMode(theme, Vim.Types.Normal);

  let borderColor = active ? modeColor : Colors.transparentBlack;
  open UiFont;

  let containerStyle =
    Style.[
      overflow(`Hidden),
      paddingHorizontal(5),
      backgroundColor(theme.editorBackground),
      borderTop(~color=Colors.transparentBlack, ~width=2),
      borderBottom(~color=borderColor, ~width=2),
      height(30),
      minWidth(minWidth_),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];

  let textStyle =
    Style.[
      textOverflow(`Ellipsis),
      fontFamily(active ? uiFont.fontFileSemiBold : uiFont.fontFile),
      fontSize(uiFont.fontSize),
      color(theme.tabActiveForeground),
      backgroundColor(theme.editorBackground),
      justifyContent(`Center),
      alignItems(`Center),
    ];

  <View style=containerStyle>
    <Clickable
      onClick
      style=Style.[
        flexGrow(1),
        flexDirection(`Row),
        alignItems(`Center),
        justifyContent(`Center),
      ]>
      <Text style=textStyle text=title />
    </Clickable>
  </View>;
};
