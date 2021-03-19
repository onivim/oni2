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

  let heading = theme => [
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
    backgroundColor(Colors.SideBar.background.from(theme)),
    height(Constants.tabHeight),
  ];

  // Minor adjustment to align with text
  let folder = [marginTop(4)];

  let item = [flexDirection(`Row), flexGrow(1), alignItems(`Center)];

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
    // Minor adjustment to align with seti-icon
    marginTop(4),
    textWrap(TextWrapping.NoWrap),
  ];
};

let nodeView =
    (
      ~isFocus,
      ~isActive,
      ~font: UiFont.t,
      ~theme,
      ~icon,
      ~node: FsTreeNode.metadata,
      ~decorations=[],
      (),
    ) => {
  let decoration =
    switch (decorations) {
    | [last, ..._] => Some(last)
    | [] => None
    };

  let tooltipText = {
    let path = node.path;
    switch (decoration) {
    | Some(decoration: Feature_Decorations.Decoration.t) =>
      FpExp.toString(path) ++ " • " ++ decoration.tooltip
    | None => FpExp.toString(path)
    };
  };

  <Tooltip text=tooltipText style=Styles.item>
    icon
    <Text
      text={node.displayName}
      style={Styles.text(~isFocus, ~isActive, ~decoration, ~theme)}
      fontFamily={font.family}
      fontSize=12.
    />
  </Tooltip>;
};

let getFileIcon = Model.getFileIcon;

let make =
    (
      ~rootName,
      ~isFocused,
      ~iconTheme,
      ~languageInfo,
      ~focusedIndex,
      ~treeView:
         Component_VimTree.model(FsTreeNode.metadata, FsTreeNode.metadata),
      ~active: option(FpExp.t(FpExp.absolute)),
      ~theme,
      ~decorations: Feature_Decorations.model,
      ~font: UiFont.t,
      ~expanded,
      ~onRootClicked: unit => unit,
      ~dispatch: Model.msg => unit,
      (),
    ) => {
  <Component_Accordion.VimTree
    title=rootName
    showCount=false
    isFocused
    focusedIndex
    expanded
    theme
    uiFont=font
    model=treeView
    dispatch={msg => dispatch(Tree(msg))}
    onClick=onRootClicked
    render={(
      ~availableWidth as _,
      ~index as _,
      ~hovered as _,
      ~selected,
      item,
    ) => {
      open FsTreeNode;
      let (icon, data) =
        switch (item) {
        | Component_VimTree.Node({data, _}) => (React.empty, data)
        | Component_VimTree.Leaf({data, _}) => (
            <Oni_Components.FileIcon
              font
              iconTheme
              languageInfo
              path={FpExp.toString(data.path)}
            />,
            data,
          )
        };
      let decorations =
        Feature_Decorations.getDecorations(
          ~path=FpExp.toString(data.path),
          decorations,
        );
      <nodeView
        icon
        isFocus=selected
        isActive={Some(data.path) == active}
        font
        theme
        node=data
        decorations
      />;
    }}
  />;
};
