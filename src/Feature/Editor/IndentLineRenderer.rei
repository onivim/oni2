/*
 * IndentLineRenderer.re
 *
 * Logic for rendering indent lines in the buffer view
 */

let render:
  (
    ~context: Draw.context,
    ~buffer: Oni_Core.Buffer.t,
    ~startLine: int,
    ~endLine: int,
    ~cursorPosition: EditorCoreTypes.Location.t,
    ~colors: Colors.t,
    ~showActive: bool,
    Oni_Core.IndentationSettings.t
  ) =>
  unit;
