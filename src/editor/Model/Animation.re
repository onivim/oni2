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
  repeat: float,

  // Actual runtime values
  startTime: float,
  remainingDelay: float,
  currentTime: float,
  currentVal: float,
  iterations: int,
};

let create = (~enabled=true, ~isActive=true, ~duration=1.0,
~repeat=false, ~delay=0., ~startTime, ()) => {
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
  let newCurrentVal = v.currentVal +. (duration /. deltaT);

  if (v.currentDelay >= 0.) {
      let newDelay = currentDelay - deltaT;
      {
        ...v,
        // Should disperse remainder into next animation iterations...
        remainingDelay: max(newDelay, 0.),
      };
  } else if (newCurrentVal >= 1.) {
    if (repeat) {
      {
        ...v,
        newCurrentVal: newCurrentVal -. 1.0,
        iterations: v.iterations + 1,
      };
    } else {
      {
        ...v,
        currentVal: 1.0,
        isActive: false,
      };
    }
  } else {
    {
      ...v,
      currentVal: newCurrentVal,
    };
  }
};

let reset = (currentTime: float, v: t) => {
  {
    ...v,
    startTime: currentTime,
    currentTime,
    currentVal: 0.,
    currentDelay: 0.,
  }
};

let start = (v: t) => {
  let v = reset(v);
  {
    ...v,
    isActive: true,
  }
}

let stop = (v: t) => {
  {
    ...v,
    isActive: false,
  }
}
