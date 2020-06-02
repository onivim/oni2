/*
 * Editor.re
 *
 * Editor component - an 'Editor' encapsulates the following:
 * - a 'tabbar'
 * - an editor surface - usually a textual buffer view
 */

open Revery.UI;
open Oni_Model;

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
    GlobalContext.current().dispatch(FilesDropped({paths: paths}));

  <View onFileDropped style={Styles.container(theme)}>
    {if (state.zenMode) {
       switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
       | Some(editorGroup) => <EditorGroupView state theme editorGroup />
       | None => React.empty
       };
     } else {
       let dispatch = msg =>
         GlobalContext.current().dispatch(Actions.Layout(msg));

       <View style={Styles.container(theme)}>
         <Feature_Layout.View theme model={state.layout} dispatch>
           ...{editorGroupId =>
             switch (
               EditorGroups.getEditorGroupById(
                 state.editorGroups,
                 editorGroupId,
               )
             ) {
             | Some(editorGroup) =>
               <EditorGroupView state theme editorGroup />
             | None => React.empty
             }
           }
         </Feature_Layout.View>
       </View>;
     }}
  </View>;
};
