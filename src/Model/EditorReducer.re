open Feature_Editor;
open Editor;

let reduce = (view, action) =>
  switch ((action: Actions.t)) {
  | SelectionChanged(selection) => {...view, selection}
  | BufferUpdate({newBuffer, _})
      when Oni_Core.Buffer.getId(newBuffer) == Editor.getBufferId(view) =>
    let buffer = EditorBuffer.ofBuffer(newBuffer);
    Editor.updateBuffer(~buffer, view);
  | EditorCursorMove(id, cursors) when EditorId.equals(view.editorId, id) => {
      ...view,
      cursors,
    }
  | EditorSetScroll(id, pixelY) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToPixelY(~pixelY, view)
  | EditorScrollToLine(id, line) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToLine(~line, view)
  | EditorScrollToColumn(id, column) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToColumn(~column, view)
  | _ => view
  };
