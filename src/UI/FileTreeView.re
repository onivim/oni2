open Oni_Model;

open Revery.UI;
open Revery.UI.Components;

module Core = Oni_Core;

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];

let containerStyles =
  Style.[
    paddingLeft(16),
    paddingVertical(8),
    overflow(`Hidden),
    flexGrow(1),
  ];

let titleStyles = (fgColor, bgColor, font, fontS) =>
  Style.[
    fontSize(fontS),
    fontFamily(font),
    backgroundColor(bgColor),
    color(fgColor),
  ];

let headingStyles = (theme: Core.Theme.t) =>
  Style.[
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.sideBarBackground),
    height(Core.Constants.default.tabHeight),
  ];

let toIcon = (~character, ~color) =>
  IconTheme.IconDefinition.{fontCharacter: character, fontColor: color};

let node =
    (
      ~font,
      ~indent,
      ~onClick,
      ~primaryRootIcon,
      ~secondaryRootIcon,
      ~itemSize,
      ~foregroundColor,
      ~backgroundColor,
      ~state: State.t,
      ~item: UiTree.t,
      (),
    ) => {
  let bgColor = backgroundColor;

  let textStyles =
    Style.[
      fontSize(itemSize),
      fontFamily(font),
      color(foregroundColor),
      backgroundColor(bgColor),
    ];

  let explorerIndent =
    Core.Configuration.getValue(
      c => c.workbenchTreeIndent,
      state.configuration,
    );

  let indentStr = String.make(indent * explorerIndent, ' ');

  let icon =
    switch (item.data) {
    | {icon, secondaryIcon, _} =>
      let makeIcon = toIcon(~color=foregroundColor);
      switch (icon, secondaryIcon, item.isOpen) {
      | (Some(_), Some(secondary), true) => secondary
      | (Some(primary), Some(_), false) => primary
      | (Some(primary), None, _) => primary
      | (None, Some(secondary), _) => secondary
      | (None, None, false) => makeIcon(~character=primaryRootIcon)
      | (None, None, true) => makeIcon(~character=secondaryRootIcon)
      };
    };

  let fontFamily =
    item.data.isDirectory ? "FontAwesome5FreeSolid.otf" : "seti.ttf";

  <Clickable
    onClick={() => onClick(item)}
    style=Style.[cursor(Revery.MouseCursors.pointer)]>
    <View style=itemStyles>
      <Text text=indentStr style=textStyles />
      <FontIcon
        fontFamily
        color={icon.fontColor}
        icon={icon.fontCharacter}
        backgroundColor
      />
      <Text
        text={item.data.displayName}
        style=Style.[marginLeft(10), ...textStyles]
      />
    </View>
  </Clickable>;
};

let make =
    (
      ~title,
      ~tree: UiTree.t,
      ~onNodeClick,
      ~state: State.t,
      ~primaryRootIcon=FontAwesome.caretRight,
      ~secondaryRootIcon=FontAwesome.caretDown,
      (),
    ) => {
  let itemSize = 12;
  let fontSize = state.uiFont.fontSize;
  let font = state.uiFont.fontFile;
  let {State.theme, _} = state;

  let backgroundColor = state.theme.sideBarBackground;
  let foregroundColor = state.theme.sideBarForeground;

  <View style=Style.[flexGrow(1)]>
    <View style={headingStyles(theme)}>
      <Text
        text=title
        style={titleStyles(foregroundColor, backgroundColor, font, fontSize)}
      />
    </View>
    <ScrollView style=containerStyles>
      <TreeView tree>
        ...{(~indent, item) =>
          <node
            onClick=onNodeClick
            primaryRootIcon
            secondaryRootIcon
            font
            itemSize
            backgroundColor
            foregroundColor
            state
            indent
            item
          />
        }
      </TreeView>
    </ScrollView>
  </View>;
};
