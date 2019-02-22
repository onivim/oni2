/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.Core;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;

type tabAction = unit => unit;

let tabHeight = 35;
let maxWidth = 165;
let fontName = "Inter-UI-SemiBold.ttf";
let fontPixelSize = 12;

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
    /* ~title, */
    /* ~active: bool, */
    /* ~onClick: tabAction, */
    /* ~onClose: tabAction, */
    /* ~children, */

    let opacityValue = active ? 1.0 : 0.6;

    let containerStyle =
      Style.[
        overflow(LayoutTypes.Hidden),
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
        <Clickable onClick>
          <View>
            <Text style=textStyle text=title />
            {
              modified ?
                <Text text={||} style=modifiedStyles /> :
                React.listToElement([])
            }
          </View>
        </Clickable>
        <Clickable onClick=onClose style=Style.[backgroundColor(Colors.red)]>
          <View style=Style.[width(20), alignSelf(`FlexEnd)]>
            <Text
              text={||}
              style=Style.[
                color(Colors.white),
                fontFamily("FontAwesome5FreeSolid.otf"),
                fontSize(20),
              ]
            />
          </View>
        </Clickable>
      </View>,
    );
  });
