/*
 * HoverView.re
 *
 */

open Revery.UI;

open Oni_Core;
open Oni_Core.Utility;
open Oni_Syntax;

module Zed_utf8 = Oni_Core.ZedBundled;
module Model = Oni_Model;
module Ext = Oni_Extensions;

open Ext.CompletionItemKind;
open Model.Completions;

let kindToIcon =
  fun
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
  | _ => FontAwesome.code;

let kindToColor =
    (
      ~textColor,
      ~functionColor,
      ~entityColor,
      ~typeColor,
      ~keywordColor,
      ~constantColor,
      default,
    ) =>
  fun
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
  | _ => default;

let completionKindToIcon: option(Ext.CompletionItemKind.t) => int =
  maybeCompletionKind => {
    maybeCompletionKind
    |> Option.map(kindToIcon)
    |> Option.value(~default=FontAwesome.question);
  };

let completionKindToColor =
    (
      default: Revery.Color.t,
      theme: TokenTheme.t,
      maybeKind: option(Ext.CompletionItemKind.t),
    ) => {
  let textColor = TokenTheme.getTextColor(theme);
  let constantColor = TokenTheme.getConstantColor(theme);
  let keywordColor = TokenTheme.getKeywordColor(theme);
  let entityColor = TokenTheme.getEntityColor(theme);
  let functionColor = TokenTheme.getFunctionColor(theme);
  let typeColor = TokenTheme.getTypeColor(theme);

  let toColor =
    kindToColor(
      ~textColor,
      ~constantColor,
      ~keywordColor,
      ~entityColor,
      ~functionColor,
      ~typeColor,
      default,
    );

  maybeKind |> Option.map(toColor) |> Option.value(~default);
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
        textWrap(Revery.TextWrapping.NoWrap),
        textOverflow(`Ellipsis),
        fontFamily(editorFont.fontFile),
        fontSize(editorFont.fontSize),
        color(highlighted ? theme.oniNormalModeBackground : fgColor),
        backgroundColor(bgColor),
      ];

    let detailTextStyle =
      Style.[
        textOverflow(`Ellipsis),
        fontFamily(editorFont.fontFile),
        fontSize(editorFont.fontSize),
        color(commentColor),
        backgroundColor(bgColor),
        margin(3),
      ];

    let lineHeight_ = lineHeight;
    let innerPositionStyle = width_ =>
      Style.[
        position(`Absolute),
        top(int_of_float(lineHeight_ +. 0.5)),
        left(0),
        width(width_),
        flexDirection(`Column),
        alignItems(`FlexStart),
        justifyContent(`Center),
        border(~color=borderColor, ~width=1),
        backgroundColor(bgColor),
      ];

    let maxCompletionWidth = 225;
    let maxDetailWidth = 225;

    let lineHeightInt = int_of_float(lineHeight_ +. 0.5);

    let detailStyle = width_ =>
      Style.[
        position(`Absolute),
        left(width_),
        top(lineHeightInt),
        width(maxDetailWidth),
        flexDirection(`Column),
        alignItems(`FlexStart),
        justifyContent(`Center),
        border(~color=borderColor, ~width=1),
        backgroundColor(bgColor),
      ];

    let (_maxWidth, completionItems) =
      List.fold_left(
        (acc, curr: Model.Filter.result(Model.CompletionItem.t)) => {
          let (prevWidth, prevDiags) = acc;

          let message = curr.item.label;
          let width =
            EditorFont.measure(~text=message, editorFont)
            +. 0.5
            |> int_of_float;

          let newWidth = max(prevWidth, width + padding);
          let completionColor =
            completionKindToColor(fgColor, state.tokenTheme, curr.item.kind);

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
                  icon={completionKindToIcon(curr.item.kind)}
                  backgroundColor=completionColor
                  color=bgColor
                  margin=4
                  // Not sure why, but specifying a font size fails to render the icon!
                  // Might be a bug with Revery font loading / re - rendering in this case?
                />
              </View>
              <View style=Style.[flexGrow(1), margin(4)]>
                <HighlightText
                  highlights={curr.highlight}
                  style=normalStyle
                  highlightStyle
                  text=message
                />
              </View>
            </View>;
          let newDiags = [newElem, ...prevDiags];
          (newWidth, newDiags);
        },
        (maxCompletionWidth, []),
        completions.filteredCompletions,
      );

    let totalWidth = _maxWidth + padding * 2;

    let detailElem =
      completions
      |> Model.Completions.getBestCompletion
      |> Option.bind(
           (filteredCompletion: Model.Completions.filteredCompletion) =>
           filteredCompletion.item.detail
         )
      |> Option.bind(text =>
           if (String.length(text) > 0) {
             Some(
               <View style={detailStyle(totalWidth)}>
                 <Text style=detailTextStyle text />
               </View>,
             );
           } else {
             None;
           }
         )
      |> Option.value(~default=React.empty);

    let completionItems = List.rev(completionItems) |> React.listToElement;

    <View style=outerPositionStyle>
      <Opacity opacity>
        <View style={innerPositionStyle(totalWidth)}> completionItems </View>
        detailElem
      </Opacity>
    </View>;
  };
};
