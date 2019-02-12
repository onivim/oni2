/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.Core;
open Revery.UI;

open Oni_Core;

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
  component((_slots: React.Hooks.empty) => {
    let theme = Theme.get();
    let style = rootStyle(theme.background, theme.foreground);

    <View style>
      <View style=surfaceStyle> <Editor state /> </View>
      <View style=statusBarStyle> <StatusBar mode={state.mode} /> </View>
    </View>;
  });
