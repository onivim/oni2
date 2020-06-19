/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;

let reduce = (v: EditorGroup.t, action: Actions.t) => {
  /* Only send updates to _active_ editor */
  let editors =
    switch (v.activeEditorId, EditorGroup.getActiveEditor(v)) {
    | (Some(id), Some(e)) =>
      IntMap.add(id, EditorReducer.reduce(e, action), v.editors)
    | _ => v.editors
    };

  let v = {...v, editors};

  switch (action) {
  | EditorFont(Service_Font.FontLoaded(font)) => {
      ...v,
      editors:
        IntMap.map(
          editor => Feature_Editor.Editor.setFont(~font, editor),
          editors,
        ),
    }

  | _ => v
  };
};
