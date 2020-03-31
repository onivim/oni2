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

module Colors = Feature_Theme.Colors;

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Editor.background.from(theme)),
    flexGrow(1),
    flexDirection(`Column),
  ];
};

let make = (~state: State.t, ~theme, ()) =>
  if (state.zenMode) {
    <View style={Styles.container(theme)}>
      {switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
       | Some(editorGroup) =>
         <EditorGroupView
           state
           theme
           windowId={state.windowManager.activeWindowId}
           editorGroup
         />
       | None => React.empty
       }}
    </View>;
  } else {
    <View style={Styles.container(theme)}>
      <EditorLayoutView state theme />
    </View>;
  };
