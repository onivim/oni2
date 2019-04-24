/*
 * Selectors.re
 *
 * Helpers to map the State.t to more usable values
 */

let getActiveEditorGroup = (state: State.t) => {
  EditorGroups.getActiveEditorGroup(state.editorGroups);
};

let getEditorGroupById = (state: State.t, id) => {
  EditorGroups.getEditorGroupById(state.editorGroups, id);
};

let getActiveEditor = (editorGroup: option(EditorGroup.t)) => {
  switch (editorGroup) {
  | None => None
  | Some(v) => EditorGroup.getActiveEditor(v)
  };
};

let getBufferById = (state: State.t, id: int) => {
  Buffers.getBuffer(id, state.buffers);
};

let getBufferForEditor = (state: State.t, editor: Editor.t) => {
  Buffers.getBuffer(editor.bufferId, state.buffers);
};

let getActiveBuffer = (state: State.t) => {
  let editorOpt = state |> getActiveEditorGroup |> getActiveEditor;

  switch (editorOpt) {
  | Some(editor) => getBufferForEditor(state, editor)
  | None => None
  };
};

let getTabs = (state: State.t, editorGroup: EditorGroup.t) => {
  let activeEditorId = editorGroup.activeEditorId;
  let f = (editorId: int) => {
    let editor = EditorGroup.getEditorById(editorId, editorGroup);

    let buffer =
      switch (editor) {
      | None => None
      | Some(v) => getBufferById(state, v.bufferId)
      };

    let active =
      switch (activeEditorId) {
      | None => false
      | Some(v) => v == editorId
      };

    switch (buffer) {
    | Some(v) => Tab.ofBuffer(~buffer=v, ~active, ())
    | None => Tab.create(~id=-1, ~title="Unknown", ())
    };
  };

  editorGroup.reverseTabOrder |> List.rev |> List.map(f);
};
