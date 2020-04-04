open Oni_Core;

[@deriving show]
type t = array(marker)

and marker =
  | Modified
  | Added
  | DeletedBefore
  | DeletedAfter
  | Unmodified;

let generate: Buffer.t => option(t);

let render:
  (
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
