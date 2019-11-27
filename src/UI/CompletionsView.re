/*
 * HoverView.re
 *
 */

open Revery.UI;

open Oni_Core;
open Oni_Syntax;
module Model = Oni_Model;

module Zed_utf8 = Oni_Core.ZedBundled;

let completionKindToIcon = (v: Oni_Extensions.CompletionKind.t) => {
  switch (v) {
  | Text => FontAwesome.alignJustify
  | Method => FontAwesome.bolt
  | Function => FontAwesome.cog
  | Constructor => FontAwesome.wrench
  | Module => FontAwesome.cubes
  | Unit => FontAwesome.cubes
  | Struct => FontAwesome.cube
  | File => FontAwesome.file
  | Keyword => FontAwesome.key
  | Reference => FontAwesome.link
  | Enum => FontAwesome.sitemap
  | Constant => FontAwesome.lock
  | Property => FontAwesome.code
  | Interface => FontAwesome.plug
  | Color => FontAwesome.paintBrush
  | _ => FontAwesome.code
  };
};

let completionKindToColor =
    (
      default: Revery.Color.t,
      theme: TokenTheme.t,
      v: Oni_Extensions.CompletionKind.t,
    ) => {
  let textColor = TokenTheme.getTextColor(theme);
  let constantColor = TokenTheme.getConstantColor(theme);
  let keywordColor = TokenTheme.getKeywordColor(theme);
  let entityColor = TokenTheme.getEntityColor(theme);
  let functionColor = TokenTheme.getFunctionColor(theme);
  let typeColor = TokenTheme.getTypeColor(theme);

  switch (v) {
  | Text => textColor
  | Value => functionColor
  | Method => functionColor
  | Function => functionColor
  | Constructor => entityColor
  | Struct => typeColor
  | Module => entityColor
  | Unit => entityColor
  | Keyword => keywordColor
  | Enum => entityColor
  | Constant => constantColor
  | Property => entityColor
  | Interface => entityColor
  | _ => default
  };
};

let make = (~x: int, ~y: int, ~lineHeight: float, ~state: Model.State.t, ()) => {
  /*let hoverEnabled =
    Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/

  let completions = state.completions;
  switch (Model.Completions.isActive(completions)) {
  | false => React.empty
  | true =>
    let {theme, editorFont, completions, _}: Model.State.t = state;

    let outerPositionStyle =
      Style.[position(`Absolute), top(y - 4), left(x + 4)];

    let opacity = 1.0;

    let bgColor = theme.editorSuggestWidgetBackground;
    let fgColor = theme.editorForeground;
    let borderColor = theme.editorSuggestWidgetBorder;

    let commentColor =
      Oni_Syntax.TokenTheme.getCommentColor(state.tokenTheme);

    let padding = 8;

    let textStyle = (~highlighted) =>
      Style.[
        //width(width_),
        //height(height_),
        textWrap(Revery.TextWrapping.NoWrap),
        textOverflow(`Ellipsis),
        fontFamily(editorFont.fontFile),
        fontSize(editorFont.fontSize),
        color(highlighted ? theme.oniNormalModeBackground : fgColor),
        backgroundColor(bgColor),
      ];

    let detailStyle = width_ =>
      Style.[
        width(width_),
        //height(height_),
        textWrap(Revery.TextWrapping.NoWrap),
        textOverflow(`Ellipsis),
        fontFamily(editorFont.fontFile),
        fontSize(editorFont.fontSize),
        color(commentColor),
        backgroundColor(bgColor),
      ];

    let lineHeight_ = lineHeight;
    let innerPositionStyle = width_ =>
      Style.[
        position(`Absolute),
        top(int_of_float(lineHeight_ +. 0.5)),
        left(0),
        width(width_),
        //height(height_),
        flexDirection(`Column),
        alignItems(`FlexStart),
        justifyContent(`Center),
        border(~color=borderColor, ~width=1),
        backgroundColor(bgColor),
      ];

    let (_maxWidth, diags) =
      List.fold_left(
        (acc, curr: Model.Filter.result(Model.Actions.completionItem)) => {
          let (prevWidth, prevDiags) = acc;

          let message = curr.item.completionLabel;
          let width =
            Types.EditorFont.measure(~text=message, editorFont)
            +. 0.5
            |> int_of_float;
          let remainingWidth = 450 - width;

          let detailElem =
            switch (curr.item.completionDetail) {
            | None => React.empty
            | Some(text) when String.length(text) > 0 =>
              let detailWidth =
                Types.EditorFont.measure(~text, editorFont)
                +. 0.5
                |> int_of_float
                |> min(remainingWidth);
              <View style=Style.[flexGrow(0), margin(4)]>
                <Text style={detailStyle(detailWidth)} text />
              </View>;
            | _ => React.empty
            };

          let newWidth = max(prevWidth, width + padding);
          let completionColor =
            completionKindToColor(
              fgColor,
              state.tokenTheme,
              curr.item.completionKind,
            );

          let normalStyle = textStyle(~highlighted=false);
          let highlightStyle = textStyle(~highlighted=true);
          let newElem =
            <View
              style=Style.[flexDirection(`Row), justifyContent(`Center)]>
              <View
                style=Style.[
                  flexDirection(`Row),
                  justifyContent(`Center),
                  alignItems(`Center),
                  flexGrow(0),
                  backgroundColor(completionColor),
                  width(25),
                ]>
                <FontIcon
                  icon={completionKindToIcon(curr.item.completionKind)}
                  backgroundColor=completionColor
                  color=bgColor
                  margin=4
                  fontSize=14
                />
              </View>
              <View style=Style.[flexGrow(1), margin(4)]>
                <HighlightText highlights={curr.highlight} style=normalStyle highlightStyle=highlightStyle text=message />
              </View>
              detailElem
            </View>;
          let newDiags = [newElem, ...prevDiags];
          (newWidth, newDiags);
        },
        (450, []),
        completions.filteredCompletions,
      );

    let diags = List.rev(diags) |> React.listToElement;

    <View style=outerPositionStyle>
      <Opacity opacity>
        <View style={innerPositionStyle(_maxWidth + padding * 2)}>
          diags
        </View>
      </Opacity>
    </View>;
  };
};
