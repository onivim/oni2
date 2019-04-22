/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Oni_Model;
module Model = Oni_Model;

module Window = WindowManager;

let noop = () => ();

let component = React.component("EditorGroupView");

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

let toUiTabs = (tabs: list(Model.Tab.t)) => {
  let f = (t: Model.Tab.t) => {
    let ret: Tabs.tabInfo = {
      title: t.title,
      modified: t.modified,
      active: t.active,
      onClick: () => GlobalContext.current().openFileById(t.id),
      onClose: () => GlobalContext.current().closeFileById(t.id),
    };
    ret;
  };

  List.map(f, tabs);
};

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let mode = state.mode;

    let editor = Selectors.getActiveEditor(state);
    let tabs = Model.Selectors.getTabs(state, state.editors)
        |> toUiTabs;
    let uiFont = state.uiFont;
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    let editorView = switch (editor)  {
    | Some(v) => <EditorSurface editor=v state />
    | None => React.empty
    };


    (
      hooks,
      <View style>
        <Tabs theme tabs mode uiFont />
        editorView
      </View>,
    );
  });
