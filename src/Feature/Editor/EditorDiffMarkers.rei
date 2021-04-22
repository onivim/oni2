open Oni_Core;

// let generate: (~scm: Feature_SCM.model, Buffer.t) => option(DiffMarkers.t);

let render:
  (
    ~context: Draw.context,
    ~rowHeight: float,
    ~x: float,
    ~width: float,
    ~canvasContext: Revery.Draw.CanvasContext.t,
    ~colors: Colors.t,
    DiffMarkers.t
  ) =>
  unit;

let renderMinimap:
  (
    ~editor: Editor.t,
    ~scrollY: float,
    ~rowHeight: float,
    ~x: float,
    ~height: float,
    ~width: float,
    ~count: int,
    ~canvasContext: Revery.Draw.CanvasContext.t,
    ~colors: Colors.t,
    DiffMarkers.t
  ) =>
  unit;
