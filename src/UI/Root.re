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

  let surface = statusBarHeight => Style.[flexGrow(1)];

  let statusBar = statusBarHeight =>
    Style.[
      backgroundColor(Color.hex("#21252b")),
      height(statusBarHeight),
      justifyContent(`Center),
      alignItems(`Center),
    ];
};

let make = (~state: State.t, ()) => {
  let theme = state.theme;
  let configuration = state.configuration;

  let statusBarVisible =
    Selectors.getActiveConfigurationValue(state, c =>
      c.workbenchStatusBarVisible
    )
    && !state.zenMode;
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
      <SearchPane state=searchPane font={state.uiFont} theme />

    | None => React.empty
    };

  <View style={Styles.root(theme.background, theme.foreground)}>
    <View style={Styles.surface(statusBarHeight)}>
      <EditorView state />
      searchPane
    </View>
    <Overlay>
      {switch (state.quickmenu) {
       | None => React.empty
       | Some(quickmenu) =>
         switch (quickmenu.variant) {
         | Wildmenu(_) => <WildmenuView theme configuration state=quickmenu />

         | _ =>
           <QuickmenuView
             theme
             configuration
             state=quickmenu
             font={state.uiFont}
           />
         }
       }}
      <KeyDisplayerView state />
      <NotificationsView state />
    </Overlay>
    statusBar
  </View>;
};
