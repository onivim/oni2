/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */
open Revery.Draw;

open EditorCoreTypes;
open Oni_Core;

open Helpers;

module Colors = Feature_Theme.Colors.EditorIndentGuide;

let rec getIndentLevel =
        (
          ~reverse=false,
          indentationSettings,
          buffer,
          startLine,
          endLine,
          line,
          previousIndentLevel,
        ) => {
  let lineText = Buffer.getLine(line, buffer) |> BufferLine.raw;

  /*
   * If the line isn't empty, we should use that lines indent level.
   *
   * If the line is empty, we should find the next non-blank line.
   * Then if the previous and next indent is within 1 indent level of each
   * other, take the larger one. Else, take the smaller indent level.
   *
   * If we hit the top or bottom of the buffer, just set the next line indent
   * to 0.
   */

  // TODO: Speed this up - no need to copy / allocate to check this!
  if (String.trim(lineText) != "") {
    Indentation.getLevel(indentationSettings, lineText);
  } else {
    let newLine = reverse ? line - 1 : line + 1;
    let nextLineLevel =
      if (newLine >= endLine || newLine <= startLine) {
        0;
      } else {
        getIndentLevel(
          ~reverse,
          indentationSettings,
          buffer,
          startLine,
          endLine,
          newLine,
          previousIndentLevel,
        );
      };

    if (abs(nextLineLevel - previousIndentLevel) <= 1
        && previousIndentLevel != 0
        && nextLineLevel != 0) {
      max(nextLineLevel, previousIndentLevel);
    } else {
      min(nextLineLevel, previousIndentLevel);
    };
  };
};

let paint = Skia.Paint.make();

let render =
    (
      ~context: Draw.context,
      ~buffer: Buffer.t,
      ~startLine: int,
      ~endLine: int,
      ~cursorPosition: Location.t,
      ~theme,
      ~showActive: bool,
      indentationSettings: IndentationSettings.t,
    ) => {
  /* First, render *all* indent guides */
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  let cursorLine = Index.toZeroBased(cursorPosition.line);
  let startLine = max(0, startLine);
  let endLine = min(bufferLineCount, endLine);

  let cursorLineIndentLevel = ref(0);
  let previousIndentLevel = ref(0);

  let indentationWidthInPixels =
    float(indentationSettings.tabSize) *. context.charWidth;

  for (line in startLine to endLine - 1) {
    let level =
      getIndentLevel(
        indentationSettings,
        buffer,
        startLine,
        endLine,
        line,
        previousIndentLevel^,
      );

    let (x, y) = bufferPositionToPixel(~context, line, 0);

    for (i in 0 to level - 1) {
      Skia.Paint.setColor(
        paint,
        Revery.Color.toSkia(Colors.background.from(theme)),
      );
      CanvasContext.drawRectLtwh(
        ~left=x +. indentationWidthInPixels *. float(i),
        ~top=y,
        ~width=1.,
        ~height=context.lineHeight,
        ~paint,
        context.canvasContext,
      );
    };

    previousIndentLevel := level;

    if (line == cursorLine) {
      cursorLineIndentLevel := level;
    };
  };

  /* Next, render _active_ indent guide */
  if (cursorLine < bufferLineCount && showActive) {
    let topFinished = ref(false);
    let topLine = ref(cursorLine - 1);
    let bottomLine = ref(cursorLine + 1);
    let bottomFinished = ref(false);
    let previousIndentLevel = ref(cursorLineIndentLevel^);

    while (topLine^ >= 0 && ! topFinished^) {
      let indentLevel =
        getIndentLevel(
          ~reverse=true,
          indentationSettings,
          buffer,
          startLine,
          endLine,
          topLine^,
          previousIndentLevel^,
        );

      if (indentLevel < cursorLineIndentLevel^) {
        topFinished := true;
      } else {
        decr(topLine);
        previousIndentLevel := indentLevel;
      };
    };

    previousIndentLevel := cursorLineIndentLevel^;

    while (bottomLine^ < bufferLineCount && ! bottomFinished^) {
      let indentLevel =
        getIndentLevel(
          indentationSettings,
          buffer,
          startLine,
          endLine,
          bottomLine^,
          previousIndentLevel^,
        );

      if (indentLevel < cursorLineIndentLevel^) {
        bottomFinished := true;
      } else {
        incr(bottomLine);
        previousIndentLevel := indentLevel;
      };
    };

    let (x, topY) = bufferPositionToPixel(~context, topLine^, 0);
    let (_, bottomY) = bufferPositionToPixel(~context, bottomLine^, 0);

    if (cursorLineIndentLevel^ >= 1) {
      Skia.Paint.setColor(
        paint,
        Revery.Color.toSkia(Colors.activeBackground.from(theme)),
      );
      CanvasContext.drawRectLtwh(
        ~left=
          x +. indentationWidthInPixels *. float(cursorLineIndentLevel^ - 1),
        ~top=topY +. context.lineHeight,
        ~width=1.,
        ~height=bottomY -. topY -. context.lineHeight,
        ~paint,
        context.canvasContext,
      );
    };
  };
};
