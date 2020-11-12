open Oni_Core;

[@deriving show]
type t = array(marker)

and marker =
  | Modified
  | Added
  | DeletedBefore
  | DeletedAfter
  | Unmodified;

let generate: (~scm: Feature_SCM.model, Buffer.t) => option(t);

let render:
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
    t
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
    t
  ) =>
  unit;
