open Feature_Editor;

let reduce = (view, action) =>
  switch ((action: Actions.t)) {
  | SelectionChanged(selection) => Editor.setSelection(~selection, view)
  | BufferUpdate({update, newBuffer, _})
      when Oni_Core.Buffer.getId(newBuffer) == Editor.getBufferId(view) =>
    let buffer = EditorBuffer.ofBuffer(newBuffer);
    Editor.updateBuffer(~update, ~buffer, view);
  | EditorCursorMove(id, cursors)
      when EditorId.equals(Editor.getId(view), id) =>
    Editor.setVimCursors(~cursors, view)
  | EditorScrollToLine(id, line)
      when EditorId.equals(Editor.getId(view), id) =>
    Editor.scrollToLine(~line, view)
  | EditorScrollToColumn(id, column)
      when EditorId.equals(Editor.getId(view), id) =>
    Editor.scrollToColumn(~column, view)
  | _ => view
  };
