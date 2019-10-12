/*
 * HoverView.re
 *
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

type tabAction = unit => unit;

let minWidth_ = 125;
let proportion = p => float_of_int(minWidth_) *. p |> int_of_float;

let component = React.component("Hover");

let createElement =
    (
      ~title,
      ~theme: Theme.t,
      ~uiFont: Types.UiFont.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {


    let containerStyle =
      Style.[
        border(~color=Colors.white, ~width=1),
        position(`Absolute),
        overflow(`Hidden),
        paddingHorizontal(5),
        backgroundColor(theme.editorBackground),
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        height(Constants.default.tabHeight),
        minWidth(minWidth_),
      ];

    let textStyle =
      Style.[
        textOverflow(`Ellipsis),
        fontFamily(uiFont.fontFile),
        fontSize(uiFont.fontSize),
        color(theme.tabActiveForeground),
        backgroundColor(theme.editorBackground),
        justifyContent(`Center),
        alignItems(`Center),
      ];

    (
      hooks,
      <View style=containerStyle>
          <Text style=textStyle text=title />
      </View>,
    );
  });
