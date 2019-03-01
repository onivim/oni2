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
      modified: t.modified,
      active: t.active,
      onClick: GlobalContext.current().openFile(~id=t.id),
      onClose: GlobalContext.current().closeFile(~id=t.id),
    };
    ret;
  };

  List.map(f, tabs);
};

let createElement = (~state: Oni_Core.State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;

    let tabs = toUiTabs(state.tabs);
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (hooks, <View style> <Tabs theme tabs /> <EditorSurface state /> </View>);
  });
