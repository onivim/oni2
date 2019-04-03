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

let toEditorTabs = (tabs: list(State.Tab.t)) =>
  List.map(
    (t: State.Tab.t) =>
      Tabs.{
        tabType: EditorTab,
        title: t.title,
        modified: t.modified,
        active: t.active,
        onClick: () => GlobalContext.current().openFileById(t.id),
        onClose: () => GlobalContext.current().closeFileById(t.id),
      },
    tabs,
  );

let toMessageTabs = (home: Home.t) =>
  List.map(
    (t: Home.guiTab) =>
      Tabs.{
        tabType: MessageTab,
        title: t.title,
        modified: false,
        active: t.selected,
        onClose: () => t.onClose() |> GlobalContext.current().dispatch,
        onClick: () => t.onClick() |> GlobalContext.current().dispatch,
      },
    home.tabs,
  );

let getActiveTabType = tabs =>
  List.find_opt((tab: Tabs.tabInfo) => tab.active, tabs)
  |> (
    fun
    | Some(t) => t.tabType
    | None => EditorTab
  );

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let {mode, theme, uiFont, tabs, home, _}: State.t = state;

    let editorTabs = toEditorTabs(tabs);
    let messageTabs = toMessageTabs(home);
    let tabs = List.append(messageTabs, editorTabs);

    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (
      hooks,
      <View style>
        <Tabs theme tabs mode uiFont />
        {switch (getActiveTabType(tabs)) {
         | EditorTab => <EditorSurface state />
         | MessageTab => <HomeView theme state />
         }}
      </View>,
    );
  });
