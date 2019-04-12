/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

open Revery.Draw;

open Oni_Core;
open Oni_Model;

type bufferPositionToPixel = (int, int) => (float, float);

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
      (),
    ) => {
  /* First, render *all* indent guides */
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  let startLine = max(0, startLine);
  let endLine = min(bufferLineCount, endLine);

  let indentationWidthInPixels =
    float_of_int(indentationSettings.tabSize) *. fontWidth;

  let l = ref(startLine);

  while (l^ < endLine) {
    let line = l^;
    let lineText = Buffer.getLine(buffer, line);

    let level = Oni_Core.Indentation.getLevel(indentationSettings, lineText);

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
  };

  /* Next, render _active_ indent guide */

  /* Get top line of region to render */

  if (cursorLine < bufferLineCount) {
    let activeIndentLevel =
      Buffer.getLine(buffer, cursorLine)
      |> Oni_Core.Indentation.getLevel(indentationSettings);

    let topFinished = ref(false);
    let topLine = ref(cursorLine);
    let bottomLine = ref(cursorLine);
    let bottomFinished = ref(false);

    while (topLine^ >= 0 && ! topFinished^) {
      let indentLevel =
        Buffer.getLine(buffer, topLine^)
        |> Oni_Core.Indentation.getLevel(indentationSettings);

      if (indentLevel < activeIndentLevel) {
        topFinished := true;
      } else {
        decr(topLine);
      };
    };

    while (bottomLine^ < bufferLineCount && ! bottomFinished^) {
      let indentLevel =
        Buffer.getLine(buffer, bottomLine^)
        |> Oni_Core.Indentation.getLevel(indentationSettings);

      if (indentLevel < activeIndentLevel) {
        bottomFinished := true;
      } else {
        incr(bottomLine);
      };
    };

    let (x, topY) = bufferPositionToPixel(topLine^, 0);
    let (_, bottomY) = bufferPositionToPixel(bottomLine^, 0);

    if (activeIndentLevel >= 1) {
      Shapes.drawRect(
        ~transform,
        ~x=
          x +. indentationWidthInPixels *. float_of_int(activeIndentLevel - 1),
        ~y=topY +. lineHeight,
        ~width=1.,
        ~height=bottomY -. topY -. lineHeight,
        ~color=theme.colors.editorIndentGuideActiveBackground,
        (),
      );
    };
  };
};
