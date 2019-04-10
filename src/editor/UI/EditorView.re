/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Oni_Model;

module Window = WindowManager;

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
    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {

        /* Acquire a 'getState' function */
        let getState = GlobalContext.current().getState;

        /* This function, given a `getState` function,
           wraps a function like `State.t => React.syntheticElement`
           and outputs a function that gives `unit => React.syntheticElement`
        */
        let unitFunction = (f) => {
            () => {
               let state = getState(); 
               f(state);
            }
        };

        let dock = unitFunction((state) => <Dock state />);
        let editor = unitFunction((state) => <EditorSurface state />);

          GlobalContext.current().dispatch(
            AddSplit(
              Window.createSplit(
                ~layout=VerticalLeft,
                ~width=50,
                ~component=dock,
                (),
              ),
            ),
          );
          GlobalContext.current().dispatch(
            AddSplit(
              Window.createSplit(
                ~layout=VerticalRight,
                ~component=editor,
                (),
              ),
            ),
          );
          None;
        },
        hooks,
      );

    let tabs = toUiTabs(state.tabs);
    let uiFont = state.uiFont;
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    (
      hooks,
      <View style>
        <Tabs theme tabs mode uiFont />
        <EditorSplits state />
      </View>,
    );
  });
