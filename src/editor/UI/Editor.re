/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;

let noop = () => ();

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

let toUiTabs = (tabs: list(Oni_Core.State.Tab.t)) => {
  let f = (t: Oni_Core.State.Tab.t) => {
    let ret: Tabs.tabInfo = {
      title: t.title,
      active: t.active,
      onClick: noop,
      onClose: noop,
    };
    ret;
  };

  List.map(f, tabs);
};

let make = (state: Oni_Core.State.t) =>
  component((_slots: React.Hooks.empty) => {
    let theme = Theme.get();

    let tabs = toUiTabs(state.tabs);
    let style = editorViewStyle(theme.background, theme.foreground);

    <View style>
      <Tabs tabs />
      <Commandline command={state.commandline} />
      <EditorSurface state />
    </View>;
  });

let createElement = (~state: Oni_Core.State.t, ~children as _, ()) =>
  React.element(make(state));
