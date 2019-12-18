open Oni_Model;

open Revery;
open Revery.UI;

module Core = Oni_Core;

let toNodePath = (workspace: Workspace.workspace, tree, path) =>
  Core.Log.perf("FileTreeview.toNodePath", () => {
    let localPath =
      Workspace.toRelativePath(workspace.workingDirectory, path);

    switch (FsTreeNode.findNodesByLocalPath(localPath, tree)) {
    | `Success(nodes) => Some(nodes)
    | `Partial(_)
    | `Failed => None
    };
  });

module Styles = {
  open Style;

  let container = [flexGrow(1)];

  let item = [flexDirection(`Row), alignItems(`Center)];

  let text = (~fg, ~font: Core.UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(fg),
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
      fontSize(int_of_float(float(size) *. 2.)),
      color(fg),
      width(int_of_float(float(size) *. 1.5)),
      height(int_of_float(float(size) *. 1.75)),
      textWrap(TextWrapping.NoWrap),
      marginLeft(-4),
    ]
  />;
};

let nodeView = (~font: Core.UiFont.t, ~fg, ~node: FsTreeNode.t, ()) => {
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
        color=fg
        icon={isOpen ? FontAwesome.folderOpen : FontAwesome.folder}
        backgroundColor=Colors.transparentWhite
      />
    | _ => <icon />
    };

  <View style=Styles.item>
    <icon />
    <Text text={node.displayName} style={Styles.text(~fg, ~font)} />
  </View>;
};

module TreeView = TreeView.Make(FsTreeNode.Model);

let make =
    (
      ~tree: FsTreeNode.t,
      ~focus: option(string),
      ~onNodeClick,
      ~state: State.t,
      (),
    ) => {
  [@warning "-27"]
  let State.{theme, uiFont as font, _} = state;

  let fg = state.theme.sideBarForeground;

  let focus =
    switch (focus, state.workspace) {
    | (Some(path), Some(workspace)) => toNodePath(workspace, tree, path)
    | _ => None
    };

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
      focus
      theme
      itemHeight=22
      onClick=onNodeClick>
      ...{node => <nodeView font fg node />}
    </TreeView>
  </View>;
};
