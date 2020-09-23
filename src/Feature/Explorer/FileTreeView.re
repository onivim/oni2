open Oni_Core;

open Revery;
open Revery.UI;

module FontAwesome = Oni_Components.FontAwesome;
module FontIcon = Oni_Components.FontIcon;
module Tooltip = Oni_Components.Tooltip;

module Colors = Feature_Theme.Colors;

module View = Revery.UI.View;

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  let heading = theme => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(Colors.SideBar.background.from(theme)),
    height(Constants.tabHeight),
  ];

  // Minor adjustment to align with text
  let folder = [marginTop(4)];

  let item = [
    flexDirection(`Row),
    flexGrow(1),
    alignItems(`Center),
  ];

  let text = (~isFocus, ~isActive, ~decoration, ~theme) => [
    color(
      switch (
        Option.bind(
          decoration, (decoration: Feature_Decorations.Decoration.t) =>
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
          Colors.SideBar.foreground.from(theme);
        }
      },
    ),
    marginLeft(10),
    // Minor adjustment to align with seti-icon
    marginTop(4),
    textWrap(TextWrapping.NoWrap),
  ];
};

let setiIcon = (~icon, ~fontSize, ~fg, ()) => {
  <Text
    text={FontIcon.codeToIcon(icon)}
    style=Style.[
      color(fg),
      width(int_of_float(fontSize *. 1.5)),
      height(int_of_float(fontSize *. 1.75)),
      textWrap(TextWrapping.NoWrap),
      // Minor adjustment to center vertically
      marginTop(-2),
      marginLeft(-4),
    ]
    fontFamily={Revery.Font.Family.fromFile("seti.ttf")}
    fontSize={fontSize *. 2.}
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
        fontSize={font.size}
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
      <View style=Styles.folder>
        <FontIcon
          color={Colors.SideBar.foreground.from(theme)}
          icon={isOpen ? FontAwesome.folderOpen : FontAwesome.folder}
        />
      </View>
    | _ => <icon />
    };

  let decoration =
    switch (decorations) {
    | [last, ..._] => Some(last)
    | [] => None
    };

  let tooltipText =
    switch (decoration) {
    | Some((decoration: Feature_Decorations.Decoration.t)) =>
      node.path ++ " • " ++ decoration.tooltip
    | None => node.path
    };

  <Tooltip text=tooltipText style={Styles.item}>
    <icon />
    <Text
      text={node.displayName}
      style={Styles.text(~isFocus, ~isActive, ~decoration, ~theme)}
      fontFamily={font.family}
      fontSize=12.
    />
  </Tooltip>;
};

let make =
    (
      ~scrollOffset,
      ~tree: FsTreeNode.t,
      ~treeView: Component_VimTree.model(FsTreeNode.t, FsTreeNode.t),
      ~active: option(string),
      ~focus: option(string),
      ~onNodeClick,
      ~theme,
      ~decorations: Feature_Decorations.model,
      ~font,
      ~dispatch: Model.msg => unit,
      (),
    ) => {
  //let onScrollOffsetChange = offset => dispatch(ScrollOffsetChanged(offset));

  <View style=Styles.container>
      <Component_VimTree.View
        isActive={true}
        theme
        model={treeView}
        dispatch={msg => dispatch(Tree(msg))}
        render={(
          ~availableWidth,
          ~index as _,
          ~hovered as _,
          ~focused,
          item,
        ) =>
          switch (item) {
          | Component_VimTree.Node({data, _})
          | Component_VimTree.Leaf({data, _}) =>
          let decorations =
            Feature_Decorations.getDecorations(~path=data.path, decorations);
              <nodeView
                isFocus=focused
                isActive={Some(data.path) == active}
                font
                theme
                node=data
                decorations
              />;
          }
        }
      />
  </View>;
};
