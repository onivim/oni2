open Oni_Core;
open Oni_Model;

open Revery;
open Revery.UI;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Tooltip = Oni_Components.Tooltip;

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  let title = (~fg, ~bg, ~font: UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    backgroundColor(bg),
    color(fg),
  ];

  let heading = (theme: Theme.t) => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(theme.sideBarBackground),
    height(Constants.tabHeight),
  ];

  let item = (~isFocus, ~isActive, ~theme: Theme.t) => [
    flexDirection(`Row),
    flexGrow(1),
    alignItems(`Center),
    backgroundColor(
      if (isFocus) {
        theme.listFocusBackground;
      } else if (isActive) {
        theme.listActiveSelectionBackground;
      } else {
        Colors.transparentWhite;
      },
    ),
  ];

  let text =
      (~isFocus, ~isActive, ~decoration, ~theme: Theme.t, ~font: UiFont.t) => [
    fontSize(11.),
    fontFamily(font.fontFile),
    color(
      switch (
        Option.bind(decoration, (decoration: Decoration.t) =>
          Theme.getCustomColor(decoration.color, theme)
        )
      ) {
      | Some(color) => color
      | None =>
        if (isFocus) {
          theme.listFocusForeground;
        } else if (isActive) {
          theme.listActiveSelectionForeground;
        } else {
          theme.foreground;
        }
      },
    ),
    marginLeft(10),
    marginVertical(2),
    textWrap(TextWrapping.NoWrap),
  ];
};

let setiIcon = (~icon, ~fontSize as size, ~fg, ()) => {
  <Text
    text={FontIcon.codeToIcon(icon)}
    style=Style.[
      fontFamily("seti.ttf"),
      fontSize(size *. 2.),
      color(fg),
      width(int_of_float(size *. 1.5)),
      height(int_of_float(size *. 1.75)),
      textWrap(TextWrapping.NoWrap),
      marginLeft(-4),
    ]
  />;
};

let nodeView =
    (
      ~isFocus,
      ~isActive,
      ~font: UiFont.t,
      ~theme: Theme.t,
      ~node: FsTreeNode.t,
      ~decorations=[],
      (),
    ) => {
  let icon = () =>
    switch (node.icon) {
    | Some(icon) =>
      <setiIcon
        fontSize={font.fontSize}
        fg={icon.fontColor}
        icon={icon.fontCharacter}
      />
    | None => React.empty
    };

  // TODO: Since the icon theme does not have a folder icon (yet), verride it
  // here in order to use FontAwesome. Remove when a better icon theme has been found.
  let icon = () =>
    switch (node.kind) {
    | Directory({isOpen, _}) =>
      <FontIcon
        color={theme.sideBarForeground}
        icon={isOpen ? FontAwesome.folderOpen : FontAwesome.folder}
      />
    | _ => <icon />
    };

  let decoration =
    switch (decorations) {
    | [last, ..._] => Some(last)
    | [] => None
    };

  let tooltipText =
    switch (decoration) {
    | Some((decoration: Decoration.t)) =>
      node.path ++ " • " ++ decoration.tooltip
    | None => node.path
    };

  <Tooltip text=tooltipText style={Styles.item(~isFocus, ~isActive, ~theme)}>
    <icon />
    <Text
      text={node.displayName}
      style={Styles.text(~isFocus, ~isActive, ~decoration, ~theme, ~font)}
    />
  </Tooltip>;
};

module TreeView = TreeView.Make(FsTreeNode.Model);

let make =
    (
      ~tree: FsTreeNode.t,
      ~active: option(string),
      ~focus: option(string),
      ~onNodeClick,
      ~state: State.t,
      (),
    ) => {
  [@warning "-27"]
  let State.{theme, uiFont as font, _} = state;

  let FileExplorer.{scrollOffset, _} = state.fileExplorer;
  let onScrollOffsetChange = offset =>
    GlobalContext.current().dispatch(
      FileExplorer(ScrollOffsetChanged(offset)),
    );

  <View style=Styles.container>
    <TreeView
      scrollOffset
      onScrollOffsetChange
      tree
      itemHeight=22
      onClick=onNodeClick
      arrowColor={theme.foreground}>
      ...{node => {
        let decorations =
          StringMap.find_opt(node.path, state.fileExplorer.decorations);

        <nodeView
          isFocus={Some(node.path) == focus}
          isActive={Some(node.path) == active}
          font
          theme
          node
          ?decorations
        />;
      }}
    </TreeView>
  </View>;
};
