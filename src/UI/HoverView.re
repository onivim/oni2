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
      ~x: int,
      ~y: int,
      ~state: Model.State.t,
      ~children as _,
      (),
    ) =>
  component(hooks => {

    let { theme, uiFont, hover, _}: Model.State.t = state;

    let outerContainerStyle = Style.[
      position(`Absolute),
      top(y),
      left(x),
    ];

    let opacity = Model.Hover.getOpacity(hover);
    
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

    let empty = (hooks, React.empty);

    switch (Model.HoverCollector.get(state)) {
    | None => empty
    | Some(hover) => {
      let diags = List.map((d: Model.Diagnostics.Diagnostic.t) => <Text style=textStyle text={d.message} />, hover.diagnostics);
      (hooks, 
        <View style=outerContainerStyle>
      <View style=containerStyle>
          <Opacity opacity>
          ...diags
          </Opacity>
      </View>
      </View>)
    }
    }
  });
