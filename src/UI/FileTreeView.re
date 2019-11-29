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

  let item = [flexDirection(`Row)];

  let text = (~fg, ~bg, ~font: Core.Types.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(fg),
    backgroundColor(bg),
    marginLeft(10),
    textWrap(TextWrapping.NoWrap),
  ];
};

let nodeView = (~font, ~fg, ~bg, ~state: State.t, ~node: UiTree.t, ()) => {
  let icon = () =>
    switch (node.data.icon) {
    | Some(icon) =>
      <FontIcon
        fontFamily="seti.ttf"
        color={icon.fontColor}
        icon={icon.fontCharacter}
        backgroundColor=bg
      />
    | None => React.empty
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
