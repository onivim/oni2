/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;

let noop = () => ();

let tabs: list(Tabs.tabInfo) = [
  {title: "file1.re", active: true, onClick: noop, onClose: noop},
  {title: "file2.re", active: false, onClick: noop, onClose: noop},
  {title: "file3.re", active: false, onClick: noop, onClose: noop},
];

let component = React.component("Editor");

let editorViewStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
    flexDirection(`Column),
  ];

let make = () =>
  component((_slots: React.Hooks.empty) => {
    let theme = Theme.get();

    let style = editorViewStyle(theme.background, theme.foreground);

    <View style> <Tabs tabs /> <EditorSurface /> </View>;
  });

let createElement = (~children as _, ()) => React.element(make());
