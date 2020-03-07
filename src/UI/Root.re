/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery;
open Revery.UI;
open Oni_Model;

module ContextMenu = Oni_Components.ContextMenu;
module KeyDisplayer = Oni_Components.KeyDisplayer;
module Tooltip = Oni_Components.Tooltip;

module Styles = {
  open Style;

  let root = (background, foreground) => [
    backgroundColor(background),
    color(foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    justifyContent(`Center),
    alignItems(`Stretch),
  ];

  let surface = [flexGrow(1), flexDirection(`Row)];

  let workspace = Style.[flexGrow(1), flexDirection(`Column)];

  let statusBar = statusBarHeight => [
    backgroundColor(Color.hex("#21252b")),
    height(statusBarHeight),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let titleBar = background =>
    Style.[flexGrow(0), height(22), backgroundColor(background)];
};

let make = (~state: State.t, ()) => {
  let State.{
        theme,
        configuration,
        contextMenu,
        uiFont as font,
        editorFont,
        sideBar,
        zenMode,
        pane,
        _,
      } = state;

  let onContextMenuItemSelect = item =>
    GlobalContext.current().dispatch(ContextMenuItemSelected(item));

  let statusBarVisible =
    Selectors.getActiveConfigurationValue(state, c =>
      c.workbenchStatusBarVisible
    )
    && !zenMode;

  let activityBarVisible =
    Selectors.getActiveConfigurationValue(state, c =>
      c.workbenchActivityBarVisible
    )
    && !zenMode;

  let sideBarVisible = !zenMode && sideBar.isOpen;

  let statusBarHeight = statusBarVisible ? 25 : 0;

  let statusBar =
    statusBarVisible
      ? <View style={Styles.statusBar(statusBarHeight)}>
          <StatusBar state contextMenu onContextMenuItemSelect />
        </View>
      : React.empty;

  let activityBar =
    activityBarVisible
      ? React.listToElement([
          <Dock
            theme={Feature_Theme.resolver(state.colorTheme)}
            sideBar
            pane
          />,
          <WindowHandle direction=Vertical theme />,
        ])
      : React.empty;

  let sideBar =
    sideBarVisible
      ? React.listToElement([
          <SideBarView state />,
          <WindowHandle direction=Vertical theme />,
        ])
      : React.empty;

  <View style={Styles.root(theme.editorBackground, theme.foreground)}>
    <Titlebar
      focused={state.windowIsFocused}
      maximized={state.windowIsMaximized}
      font={state.uiFont}
      title={state.windowTitle}
      theme={state.theme}
    />
    <View style=Styles.workspace>
      <View style=Styles.surface>
        activityBar
        sideBar
        <EditorView state />
      </View>
      <PaneView theme uiFont editorFont state />
    </View>
    <Overlay>
      {switch (state.quickmenu) {
       | None => React.empty
       | Some(quickmenu) =>
         switch (quickmenu.variant) {
         | Wildmenu(_) => <WildmenuView theme configuration state=quickmenu />

         | _ => <QuickmenuView theme configuration state=quickmenu font />
         }
       }}
      {switch (state.keyDisplayer) {
       | Some(model) => <KeyDisplayer model uiFont bottom=50 right=50 />
       | None => React.empty
       }}
    </Overlay>
    statusBar
    {let onClick = () =>
       GlobalContext.current().dispatch(ContextMenuOverlayClicked);

     <ContextMenu.Overlay onClick />}
    <Tooltip.Overlay theme font=uiFont />
    <Modals state />
    <Overlay> <SneakView state /> </Overlay>
  </View>;
};
