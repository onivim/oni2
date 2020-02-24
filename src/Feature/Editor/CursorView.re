open EditorCoreTypes;
open Revery;
open Revery.UI;

open Oni_Core;

let make =
    (
      ~buffer,
      ~mode: Vim.Mode.t,
      ~isActiveSplit,
      ~editorFont: EditorFont.t,
      ~cursorPosition: Location.t,
      ~editor: Editor.t,
      (),
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

  let fullCursorWidth =
    cursorCharacterWidth * int_of_float(editorFont.measuredWidth);

  let cursorWidth =
    switch (mode, isActiveSplit) {
    | (Insert, true) => 2
    | _ => fullCursorWidth
    };

  let cursorPixelY =
    int_of_float(
      editorFont.measuredHeight
      *. float(Index.toZeroBased(cursorPosition.line))
      -. editor.scrollY
      +. 0.5,
    );

  let cursorPixelX =
    int_of_float(
      editorFont.measuredWidth *. float(cursorOffset) -. editor.scrollX +. 0.5,
    );

  let style =
    Style.[
      position(`Absolute),
      top(cursorPixelY),
      left(cursorPixelX),
      height(int_of_float(editorFont.measuredHeight)),
      width(cursorWidth),
      backgroundColor(Colors.white),
    ];
  let cursorOpacity = isActiveSplit ? 0.5 : 0.25;

  <Opacity opacity=cursorOpacity> <View style /> </Opacity>;
};
