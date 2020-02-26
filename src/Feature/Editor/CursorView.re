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
  let bufferLine = Buffer.getLine(line, buffer);
  let lineCount = Buffer.getNumberOfLines(buffer);

  let (offset, characterWidth) =
    if (lineCount > 0 && line < lineCount) {
      BufferViewTokenizer.getCharacterPositionAndWidth(bufferLine, column);
    } else {
      (0, 1);
    };

  let x = float(offset) *. context.charWidth;
  let y = float(line) *. context.lineHeight +. 0.5;
  let height = context.lineHeight;
  let color = Revery.Colors.white;

  switch (mode, isActiveSplit) {
  | (Insert, true) =>
    let width = 2.;
    Draw.rect(~context, ~x, ~y, ~width, ~height, ~color);

  | _ =>
    let width = float(characterWidth) *. context.charWidth;
    Draw.rect(~context, ~x, ~y, ~width, ~height, ~color);

    switch (BufferLine.subExn(~index=offset, ~length=1, bufferLine)) {
    | text =>
      Draw.shapedText(
        ~context,
        ~x=x -. 0.5,
        ~y=y -. context.fontMetrics.ascent -. 0.5,
        ~color=theme.editorBackground,
        text,
      )
    | exception _ => ()
    };
  };
};
