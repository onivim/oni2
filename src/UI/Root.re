/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery;
open Revery.UI;
open Oni_Model;

module ContextMenu = Oni_Components.ContextMenu;

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
        _,
      } = state;

  let onContextMenuUpdate = model =>
    GlobalContext.current().dispatch(ContextMenuUpdated(model));

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

  let sideBarVisible =
    Selectors.getActiveConfigurationValue(state, c =>
      c.workbenchSideBarVisible
    )
    && !zenMode
    && SideBar.isOpen(sideBar);

  let statusBarHeight = statusBarVisible ? 25 : 0;

  let statusBar =
    statusBarVisible
      ? <View style={Styles.statusBar(statusBarHeight)}>
          <StatusBar state contextMenu onContextMenuUpdate />
        </View>
      : React.empty;

  let activityBar =
    activityBarVisible
      ? React.listToElement([
          <Dock state />,
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

  <View style={Styles.root(theme.background, theme.foreground)}>
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
      <KeyDisplayerView state />
    </Overlay>
    statusBar
    {switch (contextMenu) {
     | Some(model) =>
       let onOverlayClick = () =>
         GlobalContext.current().dispatch(ContextMenuOverlayClicked);
       let onItemSelect = item =>
         GlobalContext.current().dispatch(ContextMenuItemSelected(item));

       <ContextMenu.Overlay
         theme
         font=uiFont
         model
         onOverlayClick
         onItemSelect
       />;
     | None => React.empty
     }}
    <Modals state />
    <Overlay> <SneakView state /> </Overlay>
  </View>;
};
