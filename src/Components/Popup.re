type position = [ | `Top | `Bottom];

module Section = {
  type t = {
    element: Revery.UI.element,
    position,
  };
};

open Revery;
open Revery.UI;

let make = (
  ~x: int,
  ~topY: int,
  ~bottomY: int,
  ~sections: list(Section.t),
  ()
) => {
  <View style=Style.[
    position(`Absolute),
    top(topY),
    left(x),
    width(64),
    height(64),
    backgroundColor(Colors.red)
  ] />
};
