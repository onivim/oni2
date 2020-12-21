open Revery;
open Revery.UI;

// MODEL

open Msg;
type msg = Msg.t;

module UniqueId =
  UniqueId.Make({});

type t = {
  options: Spring.Options.t,
  spring: Spring.t,
  target: float,
  restThreshold: float,
  uniqueId: string,
  startTime: option(Revery.Time.t),
  tick: int,
};

let make = (~restThreshold=1.0, ~options=Spring.Options.default, position) => {
  options,
  target: position,
  startTime: None,
  spring: Spring.create(position, Time.now()),
  restThreshold,
  uniqueId:
    "Service_Animation.spring" ++ string_of_int(UniqueId.getUniqueId()),
  tick: 0,
};

// UPDATE

let isActive = ({spring, restThreshold, target, startTime, _}) => {
  startTime == None || Float.abs(spring.value -. target) > restThreshold;
};

let update = (msg, model) => {
  switch (msg) {
  | Tick(time) =>
    let (timeSinceStart, startTime) =
      switch (model.startTime) {
      | None => (Revery.Time.zero, time)
      | Some(start) => (Revery.Time.(time - start), start)
      };
    let spring =
      Spring.tick(model.target, model.spring, model.options, timeSinceStart);
    {...model, startTime: Some(startTime), spring, tick: model.tick + 1};
  };
};

let get = ({spring, target, _} as model) =>
  if (isActive(model)) {
    Spring.(spring.value);
  } else {
    target;
  };

let getTarget = ({target, _}) => target;

let set = (~instant: bool, ~position: float, model) => {
  switch (model.startTime) {
  | None => {
      ...model,
      target: position,
      spring: Spring.create(position, Revery.Time.now()),
    }
  | Some(_) when instant => {
      ...model,
      target: position,
      spring: Spring.create(position, Revery.Time.now()),
    }

  | Some(_) => {...model, target: position}
  };
};

// SUB
let sub = model =>
  if (isActive(model) || model.startTime == None) {
    Service_Time.Sub.once(
      ~uniqueId=model.uniqueId ++ "." ++ string_of_int(model.tick),
      ~delay=Revery.Time.zero,
      ~msg=(~current) =>
      Tick(current)
    );
  } else {
    Isolinear.Sub.none;
  };
