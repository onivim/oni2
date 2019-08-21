/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery;
open Revery.UI;
open Oni_Model;

let component = React.component("Root");

let rootStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    justifyContent(`Center),
    alignItems(`Center),
  ];

let surfaceStyle = statusBarHeight =>
  Style.[
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(statusBarHeight),
  ];

let statusBarStyle = statusBarHeight =>
  Style.[
    backgroundColor(Color.hex("#21252b")),
    position(`Absolute),
    left(0),
    right(0),
    bottom(0),
    height(statusBarHeight),
    justifyContent(`Center),
    alignItems(`Center),
  ];

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let style = rootStyle(theme.background, theme.foreground);

    let statusBarVisible =
      Selectors.getActiveConfigurationValue(state, c =>
        c.workbenchStatusBarVisible
      )
      && !state.zenMode;
    let statusBarHeight = statusBarVisible ? 25 : 0;
    let statusBar =
      statusBarVisible
        ? <View style={statusBarStyle(statusBarHeight)}>
            <StatusBar height=statusBarHeight state />
          </View>
        : React.empty;

    (
      hooks,
      <View style>
        <View style={surfaceStyle(statusBarHeight)}>
          <EditorView state />
        </View>
        <Overlay>
          <CommandlineView theme command={state.commandline} />
          <WildmenuView theme wildmenu={state.wildmenu} />
          <MenuView theme menu={state.menu} font={state.uiFont} />
          <KeyDisplayerView state />
        </Overlay>
        statusBar
      </View>,
    );
  });
