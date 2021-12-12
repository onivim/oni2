open Oni_Core;
// open Utility;

module BoundingBox2d = Revery.Math.BoundingBox2d;

type t = {
  currentRect: option(BoundingBox2d.t),
  showDebugFocus: bool,
  candidateText: string,
};

let initial = {
  // currentRect: None
  currentRect: Some(BoundingBox2d.create(10., 10., 50., 50.)),
  showDebugFocus: false,
  candidateText: "",
};

[@deriving show]
type msg =
  | StopTextInput
  | TextInputAvailable([@opaque] BoundingBox2d.t);

type outmsg = Isolinear.Effect.t(msg);

let setDebugView = (~enabled, model) => {...model, showDebugFocus: enabled};

let setCandidateText = (~candidateText, ~length as _, ~start as _, model) => {
  ...model,
  candidateText,
};

let isActive = model => {
  model.candidateText != "";
};

let clear = model => {...model, candidateText: ""};

let update = (msg, model) =>
  switch (msg) {
  | StopTextInput => ({...model, currentRect: None}, Isolinear.Effect.none)
  | TextInputAvailable(bbox) => (
      {...model, currentRect: Some(bbox)},
      Service_IME.Effects.setIMEPosition(bbox),
    )
  };

let sub = (~imeBoundingArea, _ime) => {
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

    SubEx.value(~uniqueId, TextInputAvailable(bbox));
  };
};

module View = {
  // open Revery;
  open Revery.UI;
  let make = (~ime, ()) => {
    switch (ime.currentRect) {
    | Some(bbox) when ime.showDebugFocus =>
      let (x, y, w, h) = BoundingBox2d.getBounds(bbox);
      <View
        style=Style.[
          position(`Absolute),
          top(y |> int_of_float),
          left(x |> int_of_float),
          width(w -. x |> int_of_float),
          height(h -. y |> int_of_float),
          backgroundColor(Revery.Color.rgba(1.0, 0., 0., 0.5)),
        ]>
        <Text text={ime.candidateText} underlined=true />
      </View>;
    | _ => React.empty
    };
  };
};
