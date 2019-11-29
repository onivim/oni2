open Oni_Model;

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Core = Oni_Core;

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  let treeContainer = [
    paddingLeft(16),
    paddingVertical(8),
    overflow(`Hidden),
    flexGrow(1),
  ];

  let title = (~fg, ~bg, ~font: Core.Types.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    backgroundColor(bg),
    color(fg),
  ];

  let heading = (theme: Core.Theme.t) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.sideBarBackground),
    height(Core.Constants.default.tabHeight),
  ];

  let item = [
    flexDirection(`Row),
    alignItems(`Center),
  ];

  let text = (~fg, ~bg, ~font: Core.Types.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(fg),
    backgroundColor(bg),
    marginLeft(10),
    marginVertical(2),
    textWrap(TextWrapping.NoWrap),
  ];
};

let setiIcon =
    (
      ~icon,
      ~fontSize as size,
      ~bg,
      ~fg,
      (),
    ) => {
  <Text
    text={FontIcon.codeToIcon(icon)}
    style=Style.[
      fontFamily("seti.ttf"),
      fontSize(int_of_float(float(size) *. 2.)),
      color(fg),
      backgroundColor(bg),
      width(int_of_float(float(size) *. 1.5)),
      height(int_of_float(float(size) *. 1.75)),
      textWrap(TextWrapping.NoWrap),
    ]
  />;
};

let nodeView = (~font: Core.Types.UiFont.t, ~fg, ~bg, ~state: State.t, ~node: UiTree.t, ()) => {
  let icon = () =>
    switch (node.data.icon) {
    | Some(icon) =>
      <setiIcon
        fontSize=font.fontSize
        fg={icon.fontColor}
        icon={icon.fontCharacter}
        bg
      />
    | None => React.empty
    };

  // TODO: Since the icon theme does not have a folder icon (yet), verride it
  // here in order to use FontAwesome. Remove when a better icon theme has been found.
  let icon = () =>
    if (node.data.isDirectory) {
      <FontIcon
        color=fg
        icon={node.isOpen ? FontAwesome.folderOpen : FontAwesome.folder}
        backgroundColor=bg
      />
    } else {
      <icon />
    };

  <View style=Styles.item>
    <icon />
    <Text text={node.data.displayName} style={Styles.text(~fg, ~bg, ~font)} />
  </View>;
};

let make = (~title, ~tree: UiTree.t, ~onNodeClick, ~state: State.t, ()) => {
  let State.{theme, uiFont as font, _} = state;

  let bg = state.theme.sideBarBackground;
  let fg = state.theme.sideBarForeground;

  <View style=Styles.container>
    <View style={Styles.heading(theme)}>
      <Text text=title style={Styles.title(~fg, ~bg, ~font)} />
    </View>
    <ScrollView style=Styles.treeContainer>
      <TreeView tree onClick=onNodeClick>
        ...{node => <nodeView font bg fg state node />}
      </TreeView>
    </ScrollView>
  </View>;
};
