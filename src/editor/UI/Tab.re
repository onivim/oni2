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
let maxWidth = 145;
let fontName = "Inter-UI-SemiBold.ttf";
let fontPixelSize = 12;

let component = React.component("Tab");

let createElement =
    (~title, ~active, ~onClick, ~onClose, ~theme: Theme.t, ~children as _, ()) =>
  component(hooks => {
    /* ~title, */
    /* ~active: bool, */
    /* ~onClick: tabAction, */
    /* ~onClose: tabAction, */
    /* ~children, */
    let _ = (onClick, onClose);

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
        color(theme.editorForeground),
      ];

    (
      hooks,
      <Clickable>
        <View style=containerStyle> <Text style=textStyle text=title /> </View>
      </Clickable>,
    );
  });
