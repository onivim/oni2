/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery;
open Revery.UI;

open Oni_Model;

let component = React.component("Root");

let statusBarHeight = 25;

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

let surfaceStyle =
  Style.[
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(statusBarHeight),
  ];

let statusBarStyle =
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
    let style = rootStyle(theme.colors.background, theme.colors.foreground);

    (
      hooks,
      <View style>
        <View style=surfaceStyle> <EditorView state /> </View>
        <Overlay>
          <CommandlineView theme command={state.commandline} />
          <WildmenuView theme wildmenu={state.wildmenu} />
          <CommandPaletteView theme commandPalette={state.commandPalette} />
        </Overlay>
        <View style=statusBarStyle>
          <StatusBar
            state
          />
        </View>
      </View>,
    );
  });
