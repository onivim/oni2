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
      ~windowIsFocused,
    ) => {
  let line = Index.toZeroBased(cursorPosition.line);
  let column = Index.toZeroBased(cursorPosition.column);
  let lineCount = Buffer.getNumberOfLines(buffer);

  if (lineCount <= 0 || line >= lineCount || !isActiveSplit) {
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

    if (!windowIsFocused) {
      let width = float(characterWidth) *. context.charWidth;
      Draw.rect(~context, ~x, ~y, ~width=1., ~height, ~color=foreground);
      Draw.rect(~context, ~x, ~y, ~width, ~height=1., ~color=foreground);
      Draw.rect(
        ~context,
        ~x,
        ~y=y +. height -. 1.,
        ~width,
        ~height=1.,
        ~color=foreground,
      );
      Draw.rect(
        ~context,
        ~x=x +. width -. 1.,
        ~y,
        ~width=1.,
        ~height,
        ~color=foreground,
      );
    } else {
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
