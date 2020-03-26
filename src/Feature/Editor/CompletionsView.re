open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Syntax;
open Oni_Components;
open Utility;

open Feature_LanguageSupport.Completions;

module Zed_utf8 = Oni_Core.ZedBundled;
module Ext = Oni_Extensions;
module CompletionItem = Feature_LanguageSupport.CompletionItem;

module Colors = {
  include Feature_Theme.Colors;
  include EditorSuggestWidget;
};

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

  let innerPosition = (~height, ~width, ~lineHeight, ~theme) => [
    position(`Absolute),
    top(int_of_float(lineHeight +. 0.5)),
    left(0),
    Style.width(width),
    Style.height(height),
    border(~color=Colors.border.from(theme), ~width=1),
    backgroundColor(Colors.background.from(theme)),
  ];

  let item = (~isFocused, ~theme) => [
    isFocused
      ? backgroundColor(Colors.selectedBackground.from(theme))
      : backgroundColor(Colors.background.from(theme)),
    flexDirection(`Row),
  ];

  let icon = (~color) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    flexGrow(0),
    backgroundColor(color),
    width(25),
    padding(4),
  ];

  let label = [flexGrow(1), margin(4)];

  let text = (~highlighted=false, ~theme, ~editorFont: Service_Font.font, ()) => [
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    textWrap(TextWrapping.NoWrap),
    color(
      highlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.Editor.foreground.from(theme),
    ),
  ];

  let highlightedText = (~theme, ~editorFont: Service_Font.font) =>
    text(~highlighted=true, ~theme, ~editorFont, ());

  let detail = (~width, ~lineHeight, ~theme) => [
    position(`Absolute),
    left(width),
    top(int_of_float(lineHeight +. 0.5)),
    Style.width(Constants.maxDetailWidth),
    flexDirection(`Column),
    alignItems(`FlexStart),
    justifyContent(`Center),
    border(~color=Colors.border.from(theme), ~width=1),
    backgroundColor(Colors.background.from(theme)),
  ];

  let detailText = (~editorFont: Service_Font.font, ~tokenTheme: TokenTheme.t) => [
    textOverflow(`Ellipsis),
    fontFamily(editorFont.fontFile),
    fontSize(editorFont.fontSize),
    color(tokenTheme.commentColor),
    margin(3),
  ];
};

let itemView =
    (
      ~isFocused,
      ~text,
      ~kind,
      ~highlight,
      ~theme,
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
    |> OptionEx.flatMap(kindToColor(tokenTheme))
    |> Option.value(~default=Colors.Editor.foreground.from(theme));

  <View style={Styles.item(~isFocused, ~theme)}>
    <View style={Styles.icon(~color=iconColor)}>
      <FontIcon
        icon
        color={Colors.background.from(theme)}
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
    <Text style={Styles.detailText(~editorFont, ~tokenTheme)} text />
  </View>;

let make =
    (
      ~x: int,
      ~y: int,
      ~lineHeight: float,
      ~theme,
      ~tokenTheme,
      ~editorFont,
      ~completions,
      (),
    ) => {
  /*let hoverEnabled =
    Configuration.getValue(c => c.editorHoverEnabled, state.configuration);*/
  let items = completions.filtered;

  let maxWidth =
    items
    |> Array.fold_left(
         (maxWidth, this: Filter.result(CompletionItem.t)) => {
           let textWidth =
             Service_Font.measure(~text=this.item.label, editorFont);
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
