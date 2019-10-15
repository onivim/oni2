/*
 * HoverView.re
 *
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

let component = React.component("Hover");

let createElement =
    (
      ~hover: option(Model.Hover.t),
      ~x: int,
      ~y: int,
      ~theme: Theme.t,
      ~uiFont: Types.UiFont.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {


    let outerContainerStyle = Style.[
      position(`Absolute),
      top(y),
      left(x),
    ];

    let containerStyle =
      Style.[
        border(~color=Colors.white, ~width=1),
        position(`Absolute),
        bottom(0),
        left(0),
        backgroundColor(theme.editorBackground),
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        minWidth(100),
        minHeight(25),
      ];

    let textStyle =
      Style.[
        textWrap(TextWrapping.NoWrap),
        fontFamily(uiFont.fontFile),
        fontSize(uiFont.fontSize),
        color(theme.tabActiveForeground),
        backgroundColor(theme.editorBackground),
      ];

    switch (hover) {
    | None => 
      (hooks, 
        <View style=outerContainerStyle>
        <View style=containerStyle>
          <View>
          <Text style=textStyle text={"Empty1"} />
          </View>
          <View>
          <Text style=textStyle text={"Empty2"} />
          </View>
          <View>
          <Text style=textStyle text={"Empty3"} />
          </View>
      </View>
      </View>)
    | Some(hover) => {
      (hooks, <View style=containerStyle>
          <Text style=textStyle text={"Hello"} />
      </View>)
    }
    }
  });
