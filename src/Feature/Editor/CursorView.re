open EditorCoreTypes;

open Oni_Core;

let render =
    (
      ~context: Draw.context,
      ~buffer,
      ~mode: Vim.Mode.t,
      ~isActiveSplit,
      ~cursorPosition: Location.t,
      ~theme: Theme.t,
    ) => {
  let line = Index.toZeroBased(cursorPosition.line);
  let column = Index.toZeroBased(cursorPosition.column);
  let lineCount = Buffer.getNumberOfLines(buffer);

  if (lineCount <= 0 || line >= lineCount) {
    ();
  } else {
    let bufferLine = Buffer.getLine(line, buffer);
    let (offset, characterWidth) =
      BufferViewTokenizer.getCharacterPositionAndWidth(bufferLine, column);

    let x = float(offset) *. context.charWidth;
    let y = float(line) *. context.lineHeight +. 0.5;
    let height = context.lineHeight;
    let background = theme.editorCursorBackground;
    let foreground = theme.editorCursorForeground;

    switch (mode, isActiveSplit) {
    | (Insert, true) =>
      let width = 2.;
      Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);

    | _ =>
      let width = float(characterWidth) *. context.charWidth;
      Draw.rect(~context, ~x, ~y, ~width, ~height, ~color=foreground);

      switch (BufferLine.subExn(~index=column, ~length=1, bufferLine)) {
      | exception _
      | "" => ()
      | text when BufferViewTokenizer.isWhitespace(ZedBundled.get(text, 0)) =>
        ()
      | text =>
        Draw.shapedText(
          ~context,
          ~x=x -. 0.5,
          ~y=y -. context.fontMetrics.ascent -. 0.5,
          ~color=background,
          text,
        )
      };
    };
  };
};
