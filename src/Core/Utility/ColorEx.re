open Revery;

let mix = (~start, ~stop, ~amount) =>
  Color.{
    r: (stop.r -. start.r) *. amount +. start.r,
    g: (stop.g -. start.g) *. amount +. start.g,
    b: (stop.b -. start.b) *. amount +. start.b,
    a: (stop.a -. start.a) *. amount +. start.a,
  };
