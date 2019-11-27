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

let editorViewStyle = (background, foreground) =>
  Style.[
    backgroundColor(background),
    color(foreground),
    flexGrow(1),
    flexDirection(`Column),
  ];

let make = (~state: State.t, ()) => {
  let theme = state.theme;
  let style = editorViewStyle(theme.background, theme.foreground);

  if (state.zenMode) {
    <View style>
      <EditorGroupView
        state
        windowId={state.windowManager.activeWindowId}
        editorGroupId={state.editorGroups.activeId}
      />
    </View>;
  } else {
    <View style> <EditorLayoutView state /> </View>;
  };
};
