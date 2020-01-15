open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Syntax;
open Oni_Model;
open Oni_Components;

open Completions;

module Zed_utf8 = Oni_Core.ZedBundled;
module Ext = Oni_Extensions;
module Option = Utility.Option;

open Ext.CompletionItemKind;

module Constants = {
  let maxCompletionWidth = 225;
  let maxDetailWidth = 225;
  let itemHeight = 22;
  let maxHeight = itemHeight * 5;
  let opacity = 1.0;
  let padding = 8;
};

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

let kindToColor = (tokenTheme: TokenTheme.t) =>
  fun
  | Text => Some(tokenTheme.textColor)
  | Method => Some(tokenTheme.functionColor)
  | Function => Some(tokenTheme.functionColor)
  | Constructor => Some(tokenTheme.entityColor)
  | Struct => Some(tokenTheme.typeColor)
  | Module => Some(tokenTheme.entityColor)
  | Unit => Some(tokenTheme.entityColor)
  | Keyword => Some(tokenTheme.keywordColor)
  | Enum => Some(tokenTheme.entityColor)
  | Constant => Some(tokenTheme.constantColor)
  | Property => Some(tokenTheme.entityColor)
  | Interface => Some(tokenTheme.entityColor)
  | _ => None;

let completionKindToIcon: option(Ext.CompletionItemKind.t) => int =
  maybeCompletionKind => {
    maybeCompletionKind
    |> Option.map(kindToIcon)
    |> Option.value(~default=FontAwesome.question);
  };

module Styles = {
  open Style;

  let outerPosition = (~x, ~y) => [
    position(`Absolute),
    top(y - 4),
    left(x + 4),
  ];

  let innerPosition = (~height, ~width, ~lineHeight, ~theme: Theme.t) => [
    position(`Absolute),
    top(int_of_float(lineHeight +. 0.5)),
    left(0),
    Style.width(width),
    Style.height(height),
    border(~color=theme.editorSuggestWidgetBorder, ~width=1),
    backgroundColor(theme.editorSuggestWidgetBackground),
  ];

  let item = (~isFocused, ~theme: Theme.t) => [
    isFocused
      ? backgroundColor(theme.editorSuggestWidgetSelectedBackground)
      : backgroundColor(theme.editorSuggestWidgetBackground),
    flexDirection(`Row),
  ];

  let icon = (~color) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    flexGrow(0),
    backgroundColor(color),
    width(25),
  ];

  let label = [flexGrow(1), margin(4)];

  let text =
      (~highlighted=false, ~theme: Theme.t, ~editorFont: EditorFont.t, ()) => [
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    textWrap(TextWrapping.NoWrap),
    color(
      highlighted ? theme.oniNormalModeBackground : theme.editorForeground,
    ),
    backgroundColor(theme.editorBackground),
  ];

  let highlightedText = (~theme: Theme.t, ~editorFont: EditorFont.t) =>
    text(~highlighted=true, ~theme, ~editorFont, ());

  let detail = (~width, ~lineHeight, ~theme: Theme.t) => [
    position(`Absolute),
    left(width),
    top(int_of_float(lineHeight +. 0.5)),
    Style.width(Constants.maxDetailWidth),
    flexDirection(`Column),
    alignItems(`FlexStart),
    justifyContent(`Center),
    border(~color=theme.editorSuggestWidgetBorder, ~width=1),
    backgroundColor(theme.editorSuggestWidgetBackground),
  ];

  let detailText =
      (~editorFont: EditorFont.t, ~theme: Theme.t, ~tokenTheme: TokenTheme.t) => [
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    color(tokenTheme.commentColor),
    margin(3),
    backgroundColor(theme.editorBackground),
  ];
};

let itemView =
    (
      ~isFocused,
      ~text,
      ~kind,
      ~highlight,
      ~theme: Theme.t,
      ~tokenTheme,
      ~editorFont,
      (),
    ) => {
  let icon =
    kind
    |> Option.map(kindToIcon)
    |> Option.value(~default=FontAwesome.question);

  let iconColor =
    kind
    |> Option.bind(kindToColor(tokenTheme))
    |> Option.value(~default=theme.editorForeground);

  <View style={Styles.item(~isFocused, ~theme)}>
    <View style={Styles.icon(~color=iconColor)}>
      <FontIcon
        icon
        backgroundColor=iconColor
        color={theme.editorSuggestWidgetBackground}
        margin=4
        // Not sure why, but specifying a font size fails to render the icon!
        // Might be a bug with Revery font loading / re - rendering in this case?
      />
    </View>
    <View style=Styles.label>
      <HighlightText
        highlights=highlight
        style={Styles.text(~theme, ~editorFont, ())}
        highlightStyle={Styles.highlightedText(~theme, ~editorFont)}
        text
      />
    </View>
  </View>;
};

let detailView =
    (~text, ~width, ~lineHeight, ~editorFont, ~theme, ~tokenTheme, ()) =>
  <View style={Styles.detail(~width, ~lineHeight, ~theme)}>
    <Text style={Styles.detailText(~editorFont, ~theme, ~tokenTheme)} text />
  </View>;

let make = (~x: int, ~y: int, ~lineHeight: float, ~state: State.t, ()) => {
  /*let hoverEnabled =
    Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/
  let {theme, tokenTheme, editorFont, completions, _}: State.t = state;
  let items = completions.filtered;

  let maxWidth =
    items
    |> Array.fold_left(
         (maxWidth, this: Filter.result(CompletionItem.t)) => {
           let textWidth =
             EditorFont.measure(~text=this.item.label, editorFont);
           let thisWidth = int_of_float(textWidth +. 0.5) + Constants.padding;
           max(maxWidth, thisWidth);
         },
         Constants.maxCompletionWidth,
       );

  let width = maxWidth + Constants.padding * 2;
  let height =
    min(Constants.maxHeight, Array.length(items) * Constants.itemHeight);

  let detail =
    switch (completions.focused) {
    | Some(index) =>
      let focused: Filter.result(CompletionItem.t) = items[index];
      switch (focused.item.detail) {
      | Some(text) =>
        <detailView text width lineHeight theme tokenTheme editorFont />
      | None => React.empty
      };
    | None => React.empty
    };

  <View style={Styles.outerPosition(~x, ~y)}>
    <Opacity opacity=Constants.opacity>
      <View
        style={Styles.innerPosition(~height, ~width, ~lineHeight, ~theme)}>
        <FlatList
          rowHeight=Constants.itemHeight
          initialRowsToRender=5
          count={Array.length(items)}
          focused={completions.focused}>
          ...{index => {
            let Filter.{highlight, item} = items[index];
            let CompletionItem.{label: text, kind, _} = item;
            <itemView
              isFocused={Some(index) == completions.focused}
              text
              kind
              highlight
              theme
              tokenTheme
              editorFont
            />;
          }}
        </FlatList>
      </View>
      detail
    </Opacity>
  </View>;
};
