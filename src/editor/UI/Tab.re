/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;
open Revery.UI.Components;

open Oni_Core;

type tabAction = unit => unit;

let tabHeight = 35;
let maxWidth = 165;
let proportion = p => float_of_int(maxWidth) *. p |> int_of_float;

let component = React.component("Tab");

let createElement =
    (
      ~title,
      ~active,
      ~modified,
      ~onClick,
      ~onClose,
      ~theme: Theme.t,
      ~uiFont: Types.UiFont.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let opacityValue = active ? 1.0 : 0.6;

    let containerStyle =
      Style.[
        overflow(`Hidden),
        paddingHorizontal(5),
        backgroundColor(theme.colors.editorBackground),
        opacity(opacityValue),
        height(tabHeight),
        width(maxWidth),
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
      ];

    let textStyle =
      Style.[
        fontFamily(uiFont.fontFile),
        fontSize(uiFont.fontSize),
        color(theme.colors.tabActiveForeground),
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
          ]>
          <Text style=textStyle text=title />
        </Clickable>
        <Clickable
          onClick=onClose
          style=Style.[
            height(tabHeight),
            alignSelf(`FlexEnd),
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
