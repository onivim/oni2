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

let toEditorTabs = (state: State.t) =>
  List.map(
    (t: State.Tab.t) =>
      Tabs.{
        title: t.title,
        modified: t.modified,
        active: t.active && !state.home.isOpen,
        onClick: GlobalContext.current().openFile(~id=t.id),
        onClose: GlobalContext.current().closeFile(~id=t.id),
      },
    state.tabs,
  );

let toMessageTabs = (home: Home.t) =>
  List.map(
    (t: Home.guiTab) =>
      Tabs.{
        title: t.title,
        modified: false,
        active: home.isOpen,
        onClose: () => t.onClose() |> GlobalContext.current().dispatch,
        onClick: () => t.onClick() |> GlobalContext.current().dispatch,
      },
    home.tabs,
  );

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let {mode, theme, uiFont, _}: State.t = state;

    let editorTabs = toEditorTabs(state);
    let messageTabs = toMessageTabs(state.home);
    let tabs = List.append(messageTabs, editorTabs);

    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (
      hooks,
      <View style>
        <Tabs theme tabs mode uiFont />
        {
          state.home.isOpen ? <HomeView theme state /> : <EditorSurface state />
        }
      </View>,
    );
  });
