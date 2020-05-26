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

let make = (~state: State.t, ~theme, ()) => {
  let onFileDropped = ({paths, _}: NodeEvents.fileDropEventParams) =>
    GlobalContext.current().dispatch(Editor(FilesDropped({paths: paths})));

  <View onFileDropped style={Styles.container(theme)}>
    {if (state.zenMode) {
       switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
       | Some(editorGroup) => <EditorGroupView state theme editorGroup />
       | None => React.empty
       };
     } else {
       <EditorLayoutView state theme />;
     }}
  </View>;
};
