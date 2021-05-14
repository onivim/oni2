open Oni_Core;
open Utility;

module BoundingBox2d = Revery.Math.BoundingBox2d;

type t = {currentRect: option(BoundingBox2d.t)};

let initial = {
  // currentRect: None
  currentRect: Some(BoundingBox2d.create(10., 10., 50., 50.)),
};

[@deriving show]
type msg =
  | StopTextInput
  | TextInputAvailable([@opaque] BoundingBox2d.t);

let update = (msg, model) =>
  switch (msg) {
  | StopTextInput => {currentRect: None}
  | TextInputAvailable(bbox) => {currentRect: Some(bbox)}
  };

let sub = (~imeBoundingArea, ime) => {
  switch (imeBoundingArea) {
  | None =>
    SubEx.value(~uniqueId="Feature_Input.IME.noTextInput", StopTextInput)
  | Some(bbox) =>
    let (x, y, width, height) = BoundingBox2d.getBounds(bbox);
    let uniqueId =
      Printf.sprintf(
        "Feature_Input.IME.textInputChanged-%f-%f-%f-%f",
        x,
        y,
        width,
        height,
      );
    // HACK:

    Sdl2.TextInput.setInputRect(50, 50, 50, 50);
    SubEx.value(~uniqueId, TextInputAvailable(bbox));
  };
};

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
          width(w -. x |> int_of_float),
          height(h -. y |> int_of_float),
          backgroundColor(Revery.Colors.red),
        ]
      />;
    };
  };
};
