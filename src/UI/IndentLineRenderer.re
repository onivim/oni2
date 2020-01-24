/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

open Revery.Draw;

open Oni_Core;
open Oni_Core.Utility;

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
  let lineText = Buffer.getLine(buffer, line);

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
  if (!StringEx.isEmpty(lineText)) {
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

let render =
    (
      ~transform,
      ~buffer: Buffer.t,
      ~startLine: int,
      ~endLine: int,
      ~lineHeight: float,
      ~fontWidth: float,
      ~bufferPositionToPixel,
      ~cursorLine,
      ~theme: Theme.t,
      ~indentationSettings: IndentationSettings.t,
      ~showActive: bool,
      (),
    ) => {
  /* First, render *all* indent guides */
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  let startLine = max(0, startLine);
  let endLine = min(bufferLineCount, endLine);

  let cursorLineIndentLevel = ref(0);
  let previousIndentLevel = ref(0);

  let indentationWidthInPixels =
    float_of_int(indentationSettings.tabSize) *. fontWidth;

  let l = ref(startLine);

  while (l^ < endLine) {
    let line = l^;
    let level =
      getIndentLevel(
        indentationSettings,
        buffer,
        startLine,
        endLine,
        line,
        previousIndentLevel^,
      );

    let (x, y) = bufferPositionToPixel(line, 0);

    let i = ref(0);
    while (i^ < level) {
      Shapes.drawRect(
        ~transform,
        ~x=x +. indentationWidthInPixels *. float_of_int(i^),
        ~y,
        ~width=1.,
        ~height=lineHeight,
        ~color=theme.editorIndentGuideBackground,
        (),
      );

      incr(i);
    };

    incr(l);
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

    let (x, topY) = bufferPositionToPixel(topLine^, 0);
    let (_, bottomY) = bufferPositionToPixel(bottomLine^, 0);

    if (cursorLineIndentLevel^ >= 1) {
      Shapes.drawRect(
        ~transform,
        ~x=
          x
          +. indentationWidthInPixels
          *. float_of_int(cursorLineIndentLevel^ - 1),
        ~y=topY +. lineHeight,
        ~width=1.,
        ~height=bottomY -. topY -. lineHeight,
        ~color=theme.editorIndentGuideActiveBackground,
        (),
      );
    };
  };
};
