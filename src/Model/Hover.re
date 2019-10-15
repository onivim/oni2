/*
 * Hover.re
 *
 * This module is responsible for the types and operations
 * for the 'Hover' view
 */

open Oni_Core;
open Oni_Core.Types;

type hover = {
  bufferId: int,
  position: Position.t,
  animation: Animation.t,
};

type t = option(hover);

let empty: t = None;

let show = (~bufferId, ~position, ~currentTime, ()) => {
  let animation =
    Animation.create(~delay=1.0, ()) |> Animation.start(currentTime);

  Some({bufferId, position, animation});
};

let tick = (deltaTime, v: t) => {
  switch (v) {
  | None => None
  | Some(hover) =>
    Some({...hover, animation: Animation.tick(deltaTime, hover.animation)})
  };
};

let getOpacity = (v: t) =>
  switch (v) {
  | None => 0.
  | Some(hover) => Animation.getValue(hover.animation)
  };

let isAnimationActive = (v: t) =>
  switch (v) {
  | None => false
  | Some(hover) => Animation.isActive(hover.animation)
  };

/*let get = (~bufferId, ~position: Position.t, state: State.t) => {
    let buffer = Buffers.getBuffer(bufferId, state.buffers);

    switch (buffer) {
    | None => None
    | Some(buf) => {
      let diagnostics = Diagnostics.getDiagnosticsAtPosition(state.diagnostics, buf, position);

      switch (diagnostics) {
      | [] => None
      | _ => Some({ diagnostics: diagnostics });
      }
    }
    }
  };*/
