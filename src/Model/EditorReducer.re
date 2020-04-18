open Oni_Core;
open Feature_Editor;
open Editor;

module Internal = {
  let getMaxLineLength = (buffer: Buffer.t) => {
    let i = ref(0);
    let lines = Buffer.getNumberOfLines(buffer);

    let max = ref(0);

    while (i^ < lines) {
      let line = i^;
      // TODO: This is approximate, beacuse the length in bytes isn't actually
      // the max length. But the length in bytes is quicker to calculate.
      let length = buffer |> Buffer.getLine(line) |> BufferLine.lengthInBytes;

      if (length > max^) {
        max := length;
      };

      incr(i);
    };

    max^;
  };
};

let recalculate = (view, maybeBuffer) =>
  switch (maybeBuffer) {
  | Some(buffer) => {
      ...view,
      viewLines: Buffer.getNumberOfLines(buffer),
      maxLineLength: Internal.getMaxLineLength(buffer),
    }
  | None => view
  };

let reduce = (view, action) =>
  switch ((action: Actions.t)) {
  | SelectionChanged(selection) => {...view, selection}
  | RecalculateEditorView(buffer) => recalculate(view, buffer)
  | EditorCursorMove(id, cursors) when EditorId.equals(view.editorId, id) => {
      ...view,
      cursors,
    }
  | EditorSetScroll(id, pixelY) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToPixelY(~pixelY, view)
  | EditorScroll(id, pixelY) when EditorId.equals(view.editorId, id) =>
    Editor.scrollDeltaPixelY(~pixelY, view)
  | EditorScrollToLine(id, line) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToLine(~line, view)
  | EditorScrollToColumn(id, column) when EditorId.equals(view.editorId, id) =>
    Editor.scrollToColumn(~column, view)
  | _ => view
  };
