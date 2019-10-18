/*
 * HoverView.re
 *
 */

open Revery.UI;

open Oni_Core;
module Model = Oni_Model;

module Zed_utf8 = Oni_Core.ZedBundled;

let component = React.component("Hover");

let createElement =
    (~x: int, ~y: int, ~lineHeight: float, ~state: Model.State.t, ~children as _, ()) =>
  component(hooks => {
    let empty = (hooks, React.empty);

    /*let hoverEnabled =
      Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/

    let completions = state.completions;
    switch (Model.Completions.isActive(completions)) {
    | false => empty
    | true =>
      open Model.HoverCollector;
      let {theme, editorFont, completions, _}: Model.State.t = state;

      let outerPositionStyle =
        Style.[position(`Absolute), top(y - 4), left(x + 4)];

      let opacity = 1.0;

      let bgColor = theme.editorHoverWidgetBackground;
      let fgColor = theme.editorForeground;
      let borderColor = theme.editorHoverWidgetBorder;

      let padding = 8;
      let innerPadding = 1;

      let textStyle =
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
          top(0),
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
          (prev, curr: Model.Actions.completionItem) => {
            let (prevWidth, prevHeight, prevDiags) = prev;

            let message = curr.completionLabel;
            let width =
              Types.EditorFont.measure(~text=message, editorFont)
              +. 0.5
              |> int_of_float;
            let height =
              Types.EditorFont.getHeight(editorFont) +. 0.5 |> int_of_float;

            let newWidth = max(prevWidth, width + padding);
            let newHeight = height + prevHeight + innerPadding;
            let newElem = <Text style=textStyle text=message />;
            let newDiags = [newElem, ...prevDiags];
            (newWidth, newHeight, newDiags);
          },
          (0, 0, []),
          completions.filteredCompletions
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
    };
  });
