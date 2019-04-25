/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

open Revery.Draw;

open Oni_Core;
open Oni_Model;

type bufferPositionToPixel = (int, int) => (float, float);

let _getIndentLevel =
    (indentationSettings, buffer, endLine, line, previousIndentLevel) => {
  let lineText = Buffer.getLine(buffer, line);

  /* 
   * If the line isn't empty, we should use that lines indent settings.
   * 
   * If the line is empty, we should check the next line along.
   *  - If the next and previous lines are within one of each other, take the minimum indent level.
   *  - If they are not, drop to 0 indentation level.
   */
  if (lineText != "") {
    Oni_Core.Indentation.getLevel(indentationSettings, lineText);
  } else {
    let nextLineLevel =
      if (line + 1 == endLine) {
        0;
      } else {
        Buffer.getLine(buffer, line + 1)
        |> Oni_Core.Indentation.getLevel(indentationSettings);
      };

    if (abs(nextLineLevel - previousIndentLevel) <= 1) {
      min(nextLineLevel, previousIndentLevel)
    } else {
      0
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
      _getIndentLevel(
        indentationSettings,
        buffer,
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
        ~color=theme.colors.editorIndentGuideBackground,
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
        _getIndentLevel(
          indentationSettings,
          buffer,
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
        _getIndentLevel(
          indentationSettings,
          buffer,
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
        ~color=theme.colors.editorIndentGuideActiveBackground,
        (),
      );
    };
  };
};
