/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

open Revery;
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
      ~cursorLine as _,
      ~theme as _,
      ~indentationSettings: IndentationSettings.t,
      (),
    ) => {
  /* First, render *all* indent guides */
  let bufferLineCount = Buffer.getNumberOfLines(buffer);
  let startLine = max(0, startLine);
  let endLine = min(bufferLineCount, endLine);

  let l = ref(startLine);

  while (l^ < endLine) {
    let line = l^;
    let lineText = Buffer.getLine(buffer, line);

    let level = Oni_Core.Indentation.getLevel(indentationSettings, lineText);

    let (x, y) = bufferPositionToPixel(line, 0);
    let indentationWidthInPixels =
      float_of_int(indentationSettings.tabSize) *. fontWidth;

    let i = ref(0);
    while (i^ < level) {
      Shapes.drawRect(
        ~transform,
        ~x=x +. indentationWidthInPixels *. float_of_int(i^),
        ~y,
        ~width=1.,
        ~height=lineHeight,
        ~color=Colors.white,
        (),
      );

      incr(i);
    };

    incr(l);
  };
  /* Next, render _active_ indent guide */
};
