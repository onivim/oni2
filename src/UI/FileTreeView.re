open Oni_Core;
open Oni_Model;

open Revery;
open Revery.UI;

module Option = Utility.Option;

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
    height(Constants.default.tabHeight),
  ];

  let item = (~isFocus, ~theme: Theme.t) => [
    flexDirection(`Row),
    flexGrow(1),
    alignItems(`Center),
    backgroundColor(
      isFocus ? theme.menuSelectionBackground : theme.sideBarBackground,
    ),
  ];

  let text = (~isActive, ~theme: Theme.t, ~font: UiFont.t) => [
    fontSize(font.fontSize),
    fontFamily(font.fontFile),
    color(isActive ? theme.oniNormalModeBackground : theme.sideBarForeground),
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

let nodeView =
    (
      ~isFocus,
      ~isActive,
      ~font: UiFont.t,
      ~theme: Theme.t,
      ~node: FsTreeNode.t,
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
        backgroundColor=Colors.transparentWhite
      />
    | _ => <icon />
    };

  <View style={Styles.item(~isFocus, ~theme)}>
    <icon />
    <Text
      text={node.displayName}
      style={Styles.text(~theme, ~isActive, ~font)}
    />
  </View>;
};

module TreeView = TreeView.Make(FsTreeNode.Model);

let%component make =
              (
                ~tree: FsTreeNode.t,
                ~active: option(string),
                ~focus: option(string),
                ~onNodeClick,
                ~state: State.t,
                (),
              ) => {
  let%hook (containerRef, setContainerRef) = Hooks.ref(None);

  [@warning "-27"]
  let State.{theme, uiFont as font, _} = state;

  let FileExplorer.{scrollOffset, _} = state.fileExplorer;
  let onScrollOffsetChange = offset =>
    GlobalContext.current().dispatch(
      FileExplorer(ScrollOffsetChanged(offset)),
    );

  let onNodeClick = node => {
    Option.iter(Revery.UI.Focus.focus, containerRef);
    onNodeClick(node);
  };

  let onKeyDown = (event: NodeEvents.keyEventParams) => {
    switch (event.keycode) {
    // Enter
    | v when v == 13 =>
      GlobalContext.current().dispatch(Actions.FileExplorer(Select))

    // arrow up
    | v when v == 1073741906 =>
      GlobalContext.current().dispatch(Actions.FileExplorer(FocusPrev))

    // arrow down
    | v when v == 1073741905 =>
      GlobalContext.current().dispatch(Actions.FileExplorer(FocusNext))

    | _ => ()
    };
  };

  <View
    onKeyDown style=Styles.container ref={ref => setContainerRef(Some(ref))}>
    <TreeView
      scrollOffset onScrollOffsetChange tree itemHeight=22 onClick=onNodeClick>
      ...{node =>
        <nodeView
          isFocus={Some(node.path) == focus}
          isActive={Some(node.path) == active}
          font
          theme
          node
        />
      }
    </TreeView>
  </View>;
};
