open Oni_Model;

open UiTree;
open Revery.UI;
open Revery.UI.Components;

module Core = Oni_Core;

let component = React.component("TreeView");

let itemStyles = Style.[flexDirection(`Row), marginVertical(5)];

let containerStyles =
  Style.[
    paddingLeft(16),
    paddingVertical(8),
    overflow(`Hidden),
    flexGrow(1),
  ];

let titleStyles = (fgColor, bgColor, font) =>
  Style.[
    fontSize(14),
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

let itemRenderer =
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
      {data, status, id}: itemContent,
    ) => {
  let isOpen =
    switch (status) {
    | Open => true
    | Closed => false
    };

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
    switch (data) {
    | FileSystemNode({icon, secondaryIcon, _}) =>
      let makeIcon = toIcon(~color=foregroundColor);
      switch (icon, secondaryIcon, isOpen) {
      | (Some(_), Some(secondary), true) => secondary
      | (Some(primary), Some(_), false) => primary
      | (Some(primary), None, _) => primary
      | (None, Some(secondary), _) => secondary
      | (None, None, false) => makeIcon(~character=primaryRootIcon)
      | (None, None, true) => makeIcon(~character=secondaryRootIcon)
      };
    };

  let fontFamily =
    switch (data) {
    | FileSystemNode({isDirectory: true, _}) => "FontAwesome5FreeSolid.otf"
    | FileSystemNode({isDirectory: false, _}) => "seti.ttf"
    };

  let label =
    switch (data) {
    | FileSystemNode({displayName, _}) => displayName
    };

  <Clickable
    onClick={() => onClick(id)}
    style=Style.[cursor(Revery.MouseCursors.pointer)]>
    <View style=itemStyles>
      <Text text=indentStr style=textStyles />
      <FontIcon
        fontFamily
        color={icon.fontColor}
        icon={icon.fontCharacter}
        backgroundColor
      />
      <Text text=label style=Style.[marginLeft(10), ...textStyles] />
    </View>
  </Clickable>;
};

let createElement =
    (
      ~title,
      ~children as _,
      ~tree: UiTree.t,
      ~onNodeClick,
      ~state: State.t,
      ~primaryRootIcon=FontAwesome.caretRight,
      ~secondaryRootIcon=FontAwesome.caretDown,
      (),
    ) =>
  component(hooks => {
    let itemSize = 12;
    let font = state.uiFont.fontFile;
    let {State.theme, _} = state;

    let onClick = id =>
      UiTree.updateNode(id, tree, ())
      |> (({updated, tree, _}) => onNodeClick(updated, tree));

    let backgroundColor = state.theme.sideBarBackground;
    let foregroundColor = state.theme.sideBarForeground;

    let nodeRenderer =
      itemRenderer(
        ~onClick,
        ~primaryRootIcon,
        ~secondaryRootIcon,
        ~font,
        ~itemSize,
        ~backgroundColor,
        ~foregroundColor,
        ~state,
      );

    (
      hooks,
      <View style=Style.[flexGrow(1)]>
        <View style={headingStyles(theme)}>
          <Text
            text=title
            style={titleStyles(foregroundColor, backgroundColor, font)}
          />
        </View>
        <ScrollView style=containerStyles>
          <Tree tree nodeRenderer />
        </ScrollView>
      </View>,
    );
  });
