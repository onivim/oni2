/*
  * EditorGroup.re
  *
  * Manage a group of editors
 */

open Oni_Core;

let reduce = (~defaultFont, v: EditorGroup.t, action: Actions.t) => {
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
  | EditorSizeChanged({id, pixelWidth, pixelHeight}) => {
      ...v,
      editors:
        IntMap.update(
          id,
          Option.map(
            Feature_Editor.Editor.setSize(~pixelWidth, ~pixelHeight),
          ),
          editors,
        ),
    }
  // TEMPORARY: Needs https://github.com/onivim/oni2/pull/1627 to remove
  | BufferEnter({buffer, _}) =>
    let editorBuffer = buffer |> Feature_Editor.EditorBuffer.ofBuffer;
    let (newState, activeEditorId) =
      EditorGroup.getOrCreateEditorForBuffer(
        ~font=defaultFont,
        ~buffer=editorBuffer,
        v,
      );

    {...newState, activeEditorId: Some(activeEditorId)};
  | Command("workbench.action.nextEditor") => EditorGroup.nextEditor(v)
  | Command("workbench.action.previousEditor") =>
    EditorGroup.previousEditor(v)
  | ViewCloseEditor(id) => EditorGroup.removeEditorById(v, id)
  | ViewSetActiveEditor(id) =>
    switch (IntMap.find_opt(id, v.editors)) {
    | None => v
    | Some(_) => {...v, activeEditorId: Some(id)}
    }
  | _ => v
  };
};
