/*
 * Root.re
 *
 * Root editor component - contains all UI elements
 */

open Revery.UI;

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

let make = () =>
  component((_slots: React.Hooks.empty) => {
    let theme = Theme.get();
    let style = rootStyle(theme.background, theme.foreground);

    <View style> <Editor /> </View>;
  });

let createElement = (~children as _, ()) => React.element(make());
