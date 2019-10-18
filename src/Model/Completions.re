/*
 * Completions.re
 *
 * This module is responsible for managing completion state
 */

open Oni_Core.Types;

type completionInfo = {
  animation: Animation.t,
};

type t = option(hover);

let empty: t = None;

let show = (~bufferId, ~position, ~currentTime, ~delay, ()) => {
  let animation =
    Animation.create(~duration=0.25, ~delay, ())
    |> Animation.start(currentTime);

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
