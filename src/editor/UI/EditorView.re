/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;

open Oni_Model;

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

let toUiTabs = (tabs: list(State.Tab.t)) => {
  let f = (t: State.Tab.t) => {
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

let toGuiTab = (tabs: list(Home.guiTab)) =>
  List.map(
    (t: Home.guiTab) =>
      Tabs.{
        title: t.title,
        modified: false,
        active: true,
        onClose: () => t.onClose() |> GlobalContext.current().dispatch,
        onClick: () => t.onClick() |> GlobalContext.current().dispatch,
      },
    tabs,
  );

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let mode = state.mode;

    let tabs = toUiTabs(state.tabs);
    let guiTabs = toGuiTab(state.home.tabs);
    let uiFont = state.uiFont;
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (
      hooks,
      <View style>
        <Tabs theme tabs={List.append(guiTabs, tabs)} mode uiFont />
        {
          state.home.isOpen ? <HomeView theme state /> : <EditorSurface state />
        }
      </View>,
    );
  });
