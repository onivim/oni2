/*
 * HoverView.re
 *
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
module Model = Oni_Model;

module Zed_utf8 = Oni_Core.ZedBundled;

let component = React.component("Hover");

let createElement =
    (~x: int, ~y: int, ~state: Model.State.t, ~children as _, ()) =>
  component(hooks => {
    let empty = (hooks, React.empty);

    let hoverEnabled = Configuration.getValue(c => c.editorHoverEnabled, state.configuration);

    switch (Model.HoverCollector.get(state)) {
    | None => empty
    | Some(hoverInfo) when hoverEnabled =>
      open Model.HoverCollector;
      let {theme, editorFont, configuration, hover, _}: Model.State.t = state;

      let outerPositionStyle =
        Style.[position(`Absolute), top(y - 4), left(x + 4)];

      let opacity = Model.Hover.getOpacity(hover);

      let bgColor = theme.editorHoverWidgetBackground;
      let fgColor = theme.editorForeground;
      let borderColor = theme.editorHoverWidgetBorder;

      let padding = 8;
      let innerPadding = 1;

      let textStyle = (width_, height_) =>
        Style.[
          //width(width_),
          //height(height_),
          //textWrap(TextWrapping.NoWrap),
          textOverflow(`Ellipsis),
          fontFamily(editorFont.fontFile),
          fontSize(editorFont.fontSize),
          color(fgColor),
          backgroundColor(bgColor),
        ];

      let innerPositionStyle = (width_, height_) =>
        Style.[
          position(`Absolute),
          bottom(0),
          left(0),
          width(width_),
          height(height_),
          flexDirection(`Column),
          alignItems(`Center),
          justifyContent(`Center),
          border(~color=borderColor, ~width=1),
          backgroundColor(bgColor),
        ];

      let (_maxWidth, height, diags) =
        List.fold_left(
          (prev, curr: Model.Diagnostics.Diagnostic.t) => {
            let (prevWidth, prevHeight, prevDiags) = prev;

            let message = curr.message;
            let width =
              Types.EditorFont.measure(~text=message, editorFont)
              +. 0.5
              |> int_of_float;
            let height =
              Types.EditorFont.getHeight(editorFont) +. 0.5 |> int_of_float;

            let newWidth = max(prevWidth, width + padding);
            let newHeight = height + prevHeight + innerPadding;
            let newStyle = textStyle(newWidth, height);
            let newElem = <Text style=newStyle text=message />;
            let newDiags = [newElem, ...prevDiags];
            (newWidth, newHeight, newDiags);
          },
          (0, 0, []),
          hoverInfo.diagnostics,
        );

      let diags = List.rev(diags);
      (
        hooks,
        <View style=outerPositionStyle>
          <Opacity opacity>
            <View
              style={innerPositionStyle(
                _maxWidth + padding * 2,
                height + padding * 2,
              )}>
              ...diags
            </View>
          </Opacity>
        </View>,
      );
    | _ => empty
    };
  });
