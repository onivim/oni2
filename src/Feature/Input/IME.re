module BoundingBox2d = Revery.Math.BoundingBox2d;

type t = {currentRect: option(BoundingBox2d.t)};

let initial = {
  // currentRect: None
  currentRect: Some(BoundingBox2d.create(10., 10., 50., 50.)),
};

[@deriving show]
type msg = unit;

let update = (msg, model) => model;

let sub = _ime => Isolinear.Sub.none;

module View = {
  open Revery;
  open Revery.UI;
  let make = (~ime, ()) => {
    switch (ime.currentRect) {
    | None => React.empty
    | Some(bbox) =>
      let (x, y, w, h) = BoundingBox2d.getBounds(bbox);
      <View
        style=Style.[
          position(`Absolute),
          top(y |> int_of_float),
          left(x |> int_of_float),
          width(w |> int_of_float),
          height(h |> int_of_float),
          backgroundColor(Revery.Colors.red),
        ]
      />;
    };
  };
};
