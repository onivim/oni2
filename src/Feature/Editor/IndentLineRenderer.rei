/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

let render:
  (
    ~canvasContext: Revery.Draw.CanvasContext.t,
    ~buffer: Oni_Core.Buffer.t,
    ~startLine: int,
    ~endLine: int,
    ~lineHeight: float,
    ~fontWidth: float,
    ~bufferPositionToPixel: (int, int) => (float, float),
    ~cursorLine: int,
    ~theme: Oni_Core.Theme.t,
    ~indentationSettings: Oni_Core.IndentationSettings.t,
    ~showActive: bool,
    unit
  ) =>
  unit;
