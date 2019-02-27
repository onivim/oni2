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
let fontName = "Inter-UI-SemiBold.ttf";
let fontPixelSize = 12;
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
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let opacityValue = active ? 1.0 : 0.6;

    let containerStyle =
      Style.[
        overflow(`Hidden),
        paddingHorizontal(5),
        backgroundColor(theme.editorBackground),
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
        color(theme.tabActiveForeground),
      ];

    let modifiedStyles =
      Style.[
        color(theme.tabActiveForeground),
        marginHorizontal(5),
        fontSize(fontPixelSize),
        fontFamily("FontAwesome5FreeSolid.otf"),
      ];

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
          {modified
             ? <Text text={||} style=modifiedStyles />
             : React.listToElement([])}
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
          <Text
            text={||}
            style=Style.[
              color(theme.tabActiveForeground),
              fontFamily("FontAwesome5FreeSolid.otf"),
              fontSize(15),
            ]
          />
        </Clickable>
      </View>,
    );
  });
