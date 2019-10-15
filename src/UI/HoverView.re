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
    (~x: int, ~y: int, ~state: Model.State.t, ~children as _, ()) =>
  component(hooks => {
    let {theme, uiFont, hover, _}: Model.State.t = state;

    let outerPositionStyle = Style.[position(`Absolute), top(y), left(x)];

    let opacity = Model.Hover.getOpacity(hover);

    let innerPositionStyle =
      Style.[
        position(`Absolute),
        bottom(0),
        left(0),
      ];

    let containerStyle = Style.[
        border(~color=Colors.white, ~width=1),
        backgroundColor(theme.editorBackground),
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        width(250),
        height(50),
    ];

    let textStyle =
      Style.[
        textWrap(TextWrapping.NoWrap),
        fontFamily(uiFont.fontFile),
        fontSize(uiFont.fontSize),
        color(theme.tabActiveForeground),
        backgroundColor(theme.editorBackground),
      ];

    let empty = (hooks, React.empty);

    switch (Model.HoverCollector.get(state)) {
    | None => empty
    | Some(hover) =>
      let diags =
        List.map(
          (d: Model.Diagnostics.Diagnostic.t) =>
            <Text style=textStyle text={d.message} />,
          hover.diagnostics,
        );
      (
        hooks,
        <View style=outerPositionStyle>
          <View style=innerPositionStyle>
            <Opacity opacity>
                <View style=containerStyle>
                ...diags 
                </View>
            </Opacity>
          </View>
        </View>,
      );
    };
  });
