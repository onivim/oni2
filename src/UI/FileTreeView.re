open Oni_Model;

open Revery;
open Revery.UI;

module Core = Oni_Core;

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  /*let title = (~fg, ~bg, ~font: Core.UiFont.t) => [
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
  ];*/

  let item = [flexDirection(`Row), alignItems(`Center)];

  let text = (~fg, ~bg, ~font: Core.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(fg),
    backgroundColor(bg),
    marginLeft(10),
    marginVertical(2),
    textWrap(TextWrapping.NoWrap),
  ];
};

let setiIcon = (~icon, ~fontSize as size, ~bg, ~fg, ()) => {
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
      marginLeft(-4),
    ]
  />;
};

let nodeView = (~font: Core.UiFont.t, ~fg, ~bg, ~node: FsTreeNode.t, ()) => {
  let icon = () =>
    switch (node.icon) {
    | Some(icon) =>
      <setiIcon
        fontSize={font.fontSize}
        fg={icon.fontColor}
        icon={icon.fontCharacter}
        bg
      />
    | None => React.empty
    };

  // TODO: Since the icon theme does not have a folder icon (yet), verride it
  // here in order to use FontAwesome. Remove when a better icon theme has been found.
  let icon = () =>
    switch (node.kind) {
    | Directory({isOpen, _}) =>
      <FontIcon
        color=fg
        icon={isOpen ? FontAwesome.folderOpen : FontAwesome.folder}
        backgroundColor=bg
      />
    | _ => <icon />
    };

  <View style=Styles.item>
    <icon />
    <Text text={node.displayName} style={Styles.text(~fg, ~bg, ~font)} />
  </View>;
};

module TreeView = TreeView.Make(FsTreeNode.Model);

let make = (~title, ~tree: FsTreeNode.t, ~onNodeClick, ~state: State.t, ()) => {
  [@warning "-27"]
  let State.{theme, uiFont as font, _} = state;

  let bg = state.theme.sideBarBackground;
  let fg = state.theme.sideBarForeground;

  <View style=Styles.container>
    <TreeView tree itemHeight=20 onClick=onNodeClick>
      ...{node => <nodeView font bg fg node />}
    </TreeView>
  </View>;
};
