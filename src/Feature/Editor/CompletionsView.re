open Revery.UI;

open Oni_Core;
open Oni_Syntax;
open Oni_Components;

open Feature_LanguageSupport.Completions;

module Zed_utf8 = Oni_Core.ZedBundled;
module CompletionItem = Feature_LanguageSupport.CompletionItem;

open Exthost.CompletionKind;

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
  | Method => Codicon.symbolMethod
  | Function => Codicon.symbolFunction
  | Constructor => Codicon.symbolConstructor
  | Field => Codicon.symbolField
  | Variable => Codicon.symbolVariable
  | Class => Codicon.symbolClass
  | Struct => Codicon.symbolStruct
  | Interface => Codicon.symbolInterface
  | Module => Codicon.symbolModule
  | Property => Codicon.symbolProperty
  | Event => Codicon.symbolEvent
  | Operator => Codicon.symbolOperator
  | Unit => Codicon.symbolUnit
  | Value => Codicon.symbolValue
  | Constant => Codicon.symbolConstant
  | Enum => Codicon.symbolEnum
  | EnumMember => Codicon.symbolEnumMember
  | Keyword => Codicon.symbolKeyword
  | Text => Codicon.symbolText
  | Color => Codicon.symbolColor
  | File => Codicon.symbolFile
  | Reference => Codicon.symbolReference
  | Customcolor => Codicon.symbolColor
  | Folder => Codicon.symbolFolder
  | TypeParameter => Codicon.symbolTypeParameter
  | User => Codicon.symbolMisc
  | Issue => Codicon.symbolMisc
  | Snippet => Codicon.symbolText;

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

module Styles = {
  open Style;

  let outerPosition = (~x, ~y) => [
    position(`Absolute),
    top(y - 4),
    left(x + 4),
  ];

  let innerPosition = (~height, ~width, ~lineHeight, ~colors: Colors.t) => [
    position(`Absolute),
    top(int_of_float(lineHeight +. 0.5)),
    left(0),
    Style.width(width),
    Style.height(height),
    border(~color=colors.suggestWidgetBorder, ~width=1),
    backgroundColor(colors.suggestWidgetBackground),
  ];

  let item = (~isFocused, ~colors: Colors.t) => [
    isFocused
      ? backgroundColor(colors.suggestWidgetSelectedBackground)
      : backgroundColor(colors.suggestWidgetBackground),
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

  let text = (~highlighted=false, ~colors: Colors.t, ()) => [
    textOverflow(`Ellipsis),
    textWrap(Revery.TextWrapping.NoWrap),
    color(
      highlighted ? colors.normalModeBackground : colors.editorForeground,
    ),
  ];

  let highlightedText = (~colors) => text(~highlighted=true, ~colors, ());

  let detail = (~width, ~lineHeight, ~colors: Colors.t) => [
    position(`Absolute),
    left(width),
    top(int_of_float(lineHeight +. 0.5)),
    Style.width(Constants.maxDetailWidth),
    flexDirection(`Column),
    alignItems(`FlexStart),
    justifyContent(`Center),
    border(~color=colors.suggestWidgetBorder, ~width=1),
    backgroundColor(colors.suggestWidgetBackground),
  ];

  let detailText = (~tokenTheme: TokenTheme.t) => [
    textOverflow(`Ellipsis),
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
      ~colors: Colors.t,
      ~tokenTheme,
      ~editorFont: Service_Font.font,
      (),
    ) => {
  let icon = kind |> kindToIcon;

  let iconColor =
    kind
    |> kindToColor(tokenTheme)
    |> Option.value(~default=colors.editorForeground);

  <View style={Styles.item(~isFocused, ~colors)}>
    <View style={Styles.icon(~color=iconColor)}>
      <Codicon
        icon
        color={colors.suggestWidgetBackground}
        // Not sure why, but specifying a font size fails to render the icon!
        // Might be a bug with Revery font loading / re - rendering in this case?
      />
    </View>
    <View style=Styles.label>
      <HighlightText
        highlights=highlight
        style={Styles.text(~colors, ())}
        highlightStyle={Styles.highlightedText(~colors)}
        fontFamily={editorFont.fontFamily}
        fontSize={editorFont.fontSize}
        text
      />
    </View>
  </View>;
};

let detailView =
    (
      ~text,
      ~width,
      ~lineHeight,
      ~editorFont: Service_Font.font,
      ~colors,
      ~tokenTheme,
      (),
    ) =>
  <View style={Styles.detail(~width, ~lineHeight, ~colors)}>
    <Text
      style={Styles.detailText(~tokenTheme)}
      fontFamily={editorFont.fontFamily}
      fontSize={editorFont.fontSize}
      text
    />
  </View>;

let make =
    (
      ~x: int,
      ~y: int,
      ~lineHeight: float,
      ~colors,
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
        <detailView text width lineHeight colors tokenTheme editorFont />
      | None => React.empty
      };
    | None => React.empty
    };

  <View style={Styles.outerPosition(~x, ~y)}>
    <Opacity opacity=Constants.opacity>
      <View
        style={Styles.innerPosition(~height, ~width, ~lineHeight, ~colors)}>
        <FlatList
          rowHeight=Constants.itemHeight
          initialRowsToRender=5
          count={Array.length(items)}
          theme
          focused={completions.focused}>
          ...{index => {
            let Filter.{highlight, item} = items[index];
            let CompletionItem.{label: text, kind, _} = item;
            <itemView
              isFocused={Some(index) == completions.focused}
              text
              kind
              highlight
              colors
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
