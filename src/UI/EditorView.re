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

let make =
    (~state: State.t, ~dispatch: Oni_Model.Actions.t => unit, ~theme, ()) => {
  let onFileDropped = ({paths, _}: NodeEvents.fileDropEventParams) =>
    dispatch(Actions.FilesDropped({paths: paths}));

  <View onFileDropped style={Styles.container(theme)}>
    {if (state.zenMode) {
       switch (EditorGroups.getActiveEditorGroup(state.editorGroups)) {
       | Some(editorGroup) =>
         <EditorGroupView state theme editorGroup dispatch />
       | None => React.empty
       };
     } else {
       let layoutDispatch = msg => dispatch(Actions.Layout(msg));

       <View style={Styles.container(theme)}>
         <Feature_Layout.View
           theme model={state.layout} dispatch=layoutDispatch>
           ...{editorGroupId =>
             switch (
               EditorGroups.getEditorGroupById(
                 state.editorGroups,
                 editorGroupId,
               )
             ) {
             | Some(editorGroup) =>
               <EditorGroupView state theme editorGroup dispatch />
             | None => React.empty
             }
           }
         </Feature_Layout.View>
         <Feature_Hover.View theme model={state.hover} />
       </View>;
     }}
  </View>;
};
