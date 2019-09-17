/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

let render:
  (
    ~transform: Reglm.Mat4.t,
    ~buffer: Oni_Model.Buffer.t,
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
