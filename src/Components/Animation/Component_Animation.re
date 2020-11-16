module Spring = Spring;

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

type msg =
  | Tick({totalTime: Revery.Time.t});

let update = (msg, model) =>
  switch (msg) {
  | Tick({totalTime}) =>
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
      Tick({totalTime: current})
    );
  };
