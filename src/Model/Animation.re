/*
 * Animations.re
 *
 * Model for UI animations
 */

type t = {
  // Metadata about the animation
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

let create = (~isActive=true, ~duration=1.0, ~repeat=false, ~delay=0., ()) => {
  isActive,
  startTime: 0.,
  delay,
  duration,
  repeat,
  remainingDelay: delay,
  currentTime: 0.,
  currentVal: 0.,
  iterations: 0,
};

let show = (v: t) => {
  "Animation startTime: "
  ++ string_of_float(v.startTime)
  ++ " isActive: "
  ++ string_of_bool(v.isActive)
  ++ " currentVal: "
  ++ string_of_float(v.currentVal);
};

let getValue = (v: t) => v.currentVal;

let tick = (deltaT: float, v: t) => {
  let newCurrentVal = v.currentVal +. deltaT /. v.duration;

  if (!v.isActive) {
    v
  } else if (v.remainingDelay > 0.) {
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

let start = (currentTime: float, v: t) => {
  {
    ...v,
    isActive: true,
    startTime: currentTime,
    currentTime,
    currentVal: 0.,
    remainingDelay: 0.,
  };
};

let stop = (v: t) => {
  {...v, isActive: false};
};

let pause = (v: t) =>
  {...v, isActive: false};

let resume = (v: t) =>
  {...v, isActive: true};