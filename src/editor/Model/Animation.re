/*
 * Animations.re
 *
 * Model for UI animations
 */

open Oni_Core;
open Oni_Core.Types;
open Oni_Extensions;

open Revery;

type t = {
  // Metadata about the animation
  enabled: bool,
  isActive: bool,
  delay: float,
  duration: float,
  repeat: bool,
  // Actual runtime values
  startTime: float,
  remainingDelay: float,
  currentTime: float,
  currentVal: float,
  iterations: int,
};

let create =
    (
      ~enabled=true,
      ~isActive=true,
      ~duration=1.0,
      ~repeat=false,
      ~delay=0.,
      ~startTime,
      (),
    ) => {
  enabled,
  isActive,
  startTime,
  delay,
  duration,
  repeat,
  remainingDelay: delay,
  currentTime: startTime,
  currentVal: 0.,
  iterations: 0,
};

let tick = (deltaT: float, v: t) => {
  let newCurrentVal = v.currentVal +. v.duration /. deltaT;

  if (v.remainingDelay >= 0.) {
    let newDelay = v.remainingDelay -. deltaT;
    {
      ...v,
      // Should disperse remainder into next animation iterations...
      remainingDelay: max(newDelay, 0.),
    };
  } else if (newCurrentVal >= 1.) {
    if (v.repeat) {
      {...v, currentVal: newCurrentVal -. 1.0, iterations: v.iterations + 1};
    } else {
      {...v, currentVal: 1.0, isActive: false};
    };
  } else {
    {...v, currentVal: newCurrentVal};
  };
};

let isActive = (v: t) => v.isActive;

let reset = (currentTime: float, v: t) => {
  {
    ...v,
    startTime: currentTime,
    currentTime,
    currentVal: 0.,
    remainingDelay: 0.,
  };
};

let stop = (v: t) => {
  {...v, isActive: false};
};
