/*
 * Tab.re
 *
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;

type tabAction = unit => unit;

let tabHeight = 35;
let maxWidth = 165;
let fontName = "selawk.ttf";
let fontPixelSize = 12;
let proportion = p => float_of_int(maxWidth) *. p |> int_of_float;

let component = React.component("Tab");

let createElement =
    (
      ~title,
      ~active, /* Where is this being set? */
      ~modified,
      ~onClick,
      ~onClose,
      ~theme: Theme.t,
      ~mode: Types.Mode.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (modeColor, _) = Theme.getColorsForMode(theme, mode);
    let borderColor = active ? modeColor : Colors.transparentBlack;
    let opacityValue = 1.0;

    let containerStyle =
      Style.[
        paddingHorizontal(5),
        backgroundColor(theme.colors.editorBackground),
        borderTop(~color=borderColor, ~width=2),
        borderBottom(~color=theme.colors.editorBackground, ~width=2),
        /* backgroundColor(Colors.red), */
        opacity(opacityValue),
        height(tabHeight),
        width(maxWidth),
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ];

    let textStyle =
      Style.[
        fontFamily(fontName),
        fontSize(fontPixelSize),
        color(theme.colors.tabActiveForeground),
        backgroundColor(theme.colors.editorBackground),
      ];

    let icon = modified ? FontAwesome.circle : FontAwesome.times;

    (
      hooks,
      <View style=containerStyle>
        <Clickable
          onClick
          style=Style.[
            width(proportion(0.80)),
            flexDirection(`Row),
            alignItems(`Center),
            justifyContent(`Center),
            overflow(`Hidden),
          ]>
          <Text style=textStyle text=title />
        </Clickable>
        <Clickable
          onClick=onClose
          style=Style.[
            height(tabHeight),
            /* alignSelf(`FlexEnd), */
            alignItems(`Center),
            justifyContent(`Center),
            width(proportion(0.20)),
          ]>
          <FontIcon
            icon
            backgroundColor={theme.colors.editorBackground}
            color={theme.colors.tabActiveForeground}
            fontSize={modified ? 10 : 12}
          />
        </Clickable>
      </View>,
    );
  });
