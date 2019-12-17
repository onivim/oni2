/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery;
open Revery.UI;
open Oni_Model;

module Styles = {
  let root = (background, foreground) =>
    Style.[
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

  let surface = Style.[flexGrow(1), flexDirection(`Row)];

  let statusBar = statusBarHeight =>
    Style.[
      backgroundColor(Color.hex("#21252b")),
      height(statusBarHeight),
      justifyContent(`Center),
      alignItems(`Center),
    ];
};

let make = (~state: State.t, ()) => {
  let State.{theme, configuration, uiFont, editorFont, sideBar, zenMode, _} = state;

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
          <StatusBar height=statusBarHeight state />
        </View>
      : React.empty;

  let searchPane =
    switch (state.searchPane) {
    | Some(searchPane) =>
      <SearchPane state=searchPane uiFont editorFont theme />
    // TODO: BUG - Why is this needed? Why can't it be 'React.empty'?
    // Without this: when switching out of zen mode, the entire
    // editor surface will be empty. Seems like a reconciliation bug.
    | None => <View />
    };

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
    <View style=Styles.surface>
      activityBar
      sideBar
      <EditorView state />
      <PaneView theme uiFont editorFont state />
    </View>
    <Overlay>
      {switch (state.quickmenu) {
       | None => React.empty
       | Some(quickmenu) =>
         switch (quickmenu.variant) {
         | Wildmenu(_) => <WildmenuView theme configuration state=quickmenu />

         | _ =>
           <QuickmenuView theme configuration state=quickmenu font=uiFont />
         }
       }}
      <KeyDisplayerView state />
      <NotificationsView state />
    </Overlay>
    statusBar
  </View>;
};
