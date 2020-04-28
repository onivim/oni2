open Oni_Core;
open Oni_Model;

open Revery;
open Revery.UI;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Tooltip = Oni_Components.Tooltip;

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  let title = (~fg, ~font: UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(fg),
  ];

  let heading = theme => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(Colors.SideBar.background.from(theme)),
    height(Constants.tabHeight),
  ];

  let item = (~isFocus, ~isActive, ~theme) => [
    flexDirection(`Row),
    flexGrow(1),
    alignItems(`Center),
    backgroundColor(
      if (isActive) {
        Colors.List.activeSelectionBackground.from(theme);
      } else if (isFocus) {
        Colors.List.focusBackground.from(theme);
      } else {
        Revery.Colors.transparentWhite;
      },
    ),
  ];

  let text = (~isFocus, ~isActive, ~decoration, ~theme, ~font: UiFont.t) => [
    fontSize(11.),
    fontFamily(font.fontFile),
    color(
      switch (
        Option.bind(decoration, (decoration: Decoration.t) =>
          ColorTheme.(Colors.get(key(decoration.color), theme))
        )
      ) {
      | Some(color) => color
      | None =>
        if (isActive) {
          Colors.List.activeSelectionForeground.from(theme);
        } else if (isFocus) {
          Colors.List.focusForeground.from(theme);
        } else {
          Colors.foreground.from(theme);
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
      ~theme,
      ~node: FsTreeNode.t,
      ~decorations=[],
      (),
    ) => {
  let icon = () =>
    switch (node.icon) {
    | Some(icon) =>
      <setiIcon
        fontSize={font.fontSize}
        fg={icon.colorHex |> Revery.Color.hex}
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
        color={Colors.SideBar.foreground.from(theme)}
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
      ~scrollOffset,
      ~decorations,
      ~tree: FsTreeNode.t,
      ~active: option(string),
      ~focus: option(string),
      ~onNodeClick,
      ~theme,
      ~font,
      (),
    ) => {
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
      arrowColor={Colors.foreground.from(theme)}>
      ...{node => {
        let decorations = StringMap.find_opt(node.path, decorations);

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
