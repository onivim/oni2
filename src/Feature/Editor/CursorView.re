open EditorCoreTypes;

open Oni_Core;

let render =
    (
      ~context,
      ~buffer,
      ~mode: Vim.Mode.t,
      ~isActiveSplit,
      ~editorFont: EditorFont.t,
      ~cursorPosition: Location.t,
      ~editor: Editor.t,
    ) => {
  let cursorLine = Index.toZeroBased(cursorPosition.line);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let (cursorOffset, cursorCharacterWidth) =
    if (lineCount > 0 && cursorLine < lineCount) {
      let cursorLine = Buffer.getLine(cursorLine, buffer);

      let (cursorOffset, width) =
        BufferViewTokenizer.getCharacterPositionAndWidth(
          cursorLine,
          Index.toZeroBased(cursorPosition.column),
        );
      (cursorOffset, width);
    } else {
      (0, 1);
    };

  let x =
    editorFont.measuredWidth *. float(cursorOffset) -. editor.scrollX +. 0.5;

  let y =
    editorFont.measuredHeight
    *. float(Index.toZeroBased(cursorPosition.line))
    -. editor.scrollY
    +. 0.5;

  let fullCursorWidth =
    float(cursorCharacterWidth) *. editorFont.measuredWidth;

  let width =
    switch (mode, isActiveSplit) {
    | (Insert, true) => 2.
    | _ => fullCursorWidth
    };

  let height = editorFont.measuredHeight;

  let opacity = isActiveSplit ? 0.5 : 0.25;
  let color = Revery.Color.multiplyAlpha(opacity, Revery.Colors.white);

  Draw.rect(~context, ~x, ~y, ~width, ~height, ~color);
};
