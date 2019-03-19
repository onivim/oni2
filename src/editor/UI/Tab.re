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
let minWidth_ = 125;
let proportion = p => float_of_int(minWidth_) *. p |> int_of_float;

let component = React.component("Tab");

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

let createElement =
    (
      ~title,
      ~tabPosition,
      ~numberOfTabs,
      ~active,
      ~modified,
      ~onClick,
      ~onClose,
      ~theme: Theme.t,
      ~uiFont: Types.UiFont.t,
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
        overflow(`Hidden),
        paddingHorizontal(5),
        backgroundColor(theme.colors.editorBackground),
        borderTop(~color=borderColor, ~width=2),
        borderBottom(~color=theme.colors.editorBackground, ~width=2),
        opacity(opacityValue),
        height(tabHeight),
        minWidth(minWidth_),
        flexDirection(`Row),
        justifyContent(`Center),
        alignItems(`Center),
        ...horizontalBorderStyles(tabPosition, numberOfTabs),
      ];

    let textStyle =
      Style.[
        width(proportion(0.80) - 10),
        textOverflow(`Ellipsis),
        fontFamily(uiFont.fontFile),
        fontSize(uiFont.fontSize),
        color(theme.colors.tabActiveForeground),
        backgroundColor(theme.colors.editorBackground),
        marginLeft(5),
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
