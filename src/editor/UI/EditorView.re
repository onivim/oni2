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

let createElement = (~state: State.t, ~children as _, ()) =>
  component(hooks => {
    let theme = state.theme;
    let style =
      editorViewStyle(theme.colors.background, theme.colors.foreground);

    if (state.zenMode) {
      (
        hooks,
        <View style>
          <EditorGroupView
            state
            windowId=0
            editorGroupId={state.editorGroups.activeId}
          />
        </View>,
      );
    } else {
      (hooks, <View style> <EditorLayoutView state /> </View>);
    };
  });
