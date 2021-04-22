open Msg;
type msg = Msg.t;

module Spring = Spring;
module Animator = Animator;

open Revery;
open Revery.UI;

type t('value) = {
  uniqueId: string,
  animation: Revery.UI.Animation.t('value),
  maybeStartTime: option(Revery.Time.t),
  duration: Revery.Time.t,
  isComplete: bool,
  tick: int,
};

module UniqueId =
  UniqueId.Make({});

let make = animation => {
  uniqueId:
    "Service_Animation.animation" ++ (UniqueId.getUniqueId() |> string_of_int),
  animation,
  maybeStartTime: None,
  duration: Revery.Time.zero,
  isComplete: false,
  tick: 0,
};

let constant = v => {
  let animation = Animation.const(v);
  make(animation);
};

let update = (msg, model) =>
  switch (msg) {
  | Tick(totalTime) =>
    let (startTime, deltaTime) =
      switch (model.maybeStartTime) {
      | None => (totalTime, Time.zero)
      | Some(prev) => (prev, Time.(totalTime - prev))
      };

    let isComplete =
      switch (Animation.stateAt(deltaTime, model.animation)) {
      | Complete(_) => true
      | Delayed
      | Running => false
      };
    {
      ...model,
      maybeStartTime: Some(startTime),
      isComplete,
      duration: deltaTime,
      tick: model.tick + 1,
    };
  };
let isComplete = ({isComplete, _}) => isComplete;

let isActive = model => !isComplete(model);
let get = ({animation, duration, _}) => {
  Animation.valueAt(duration, animation);
};

let sub = ({isComplete, uniqueId, tick, _}) =>
  if (isComplete) {
    Isolinear.Sub.none;
  } else {
    Service_Time.Sub.once(
      ~uniqueId=uniqueId ++ string_of_int(tick),
      ~delay=Revery.Time.zero,
      ~msg=(~current) =>
      Tick(current)
    );
  };

module ColorTransition = {
  type nonrec t = {
    startColor: Color.t,
    stopColor: Color.t,
    duration: Time.t,
    delay: Time.t,
    animation: t(float),
  };
  type nonrec msg = msg;

  module Internal = {
    let makeAnimation = (~duration, ~delay as delayDuration) => {
      Animation.(
        animate(duration)
        |> delay(delayDuration)
        |> ease(Easing.linear)
        |> tween(0., 1.)
      )
      |> make;
    };

    let instantAnimation = Animation.const(1.0) |> make;
  };

  let make = (~duration, ~delay, color): t => {
    startColor: color,
    stopColor: color,
    duration,
    delay,
    animation: Internal.instantAnimation,
  };

  let update = (msg, {animation, _} as model) => {
    {...model, animation: update(msg, animation)};
  };

  let get = ({animation, startColor, stopColor, _}) => {
    let interp = get(animation);
    Color.mix(~start=startColor, ~stop=stopColor, ~amount=interp);
  };

  let set = (~instant, ~color, model) => {
    let startColor = get(model);
    let stopColor = color;

    let animation =
      instant
        ? Internal.instantAnimation
        : Internal.makeAnimation(~delay=model.delay, ~duration=model.duration);
    {...model, startColor, stopColor, animation};
  };

  let sub = ({animation, _}) => sub(animation);
};

let subAny = (~uniqueId) =>
  Service_Time.Sub.once(~uniqueId, ~delay=Revery.Time.zero, ~msg=(~current) =>
    Tick(current)
  );
