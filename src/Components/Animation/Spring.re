open Revery;
open Revery.UI;

// MODEL

open Msg;
type msg = Msg.t;

module UniqueId =
  UniqueId.Make({});

module PhysicalSpring = {
  type t = {
    value: float,
    velocity: float,
    acceleration: float,
  };

  let update = (~target, ~options: Spring.Options.t, deltaT, spring) => {
    let deltaT =
      if (deltaT <= 0.) {
        0.033;
      } else {
        min(deltaT, 0.033);
      };
    // Cap the delta at 33 milliseconds / 30 FPS
    // This is important if the animation has been inactive!
    let deltaT = min(deltaT, 0.033);
    let force = Float.abs(target -. spring.value) *. options.stiffness;
    let dir = spring.value > target ? (-1.) : 1.;

    let acceleration = dir *. force -. options.damping *. spring.velocity;
    let velocity = spring.velocity +. acceleration *. deltaT;
    let value = spring.value +. velocity *. deltaT;

    {acceleration, velocity, value};
  };

  let create = initialPosition => {
    value: initialPosition,
    velocity: 0.,
    acceleration: 0.,
  };
};

type t = {
  options: Spring.Options.t,
  spring: PhysicalSpring.t,
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
  spring: PhysicalSpring.create(position),
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
      PhysicalSpring.update(
        ~target=model.target,
        ~options=model.options,
        Revery.Time.toFloatSeconds(timeSinceStart),
        model.spring,
      );
    {...model, startTime: Some(startTime), spring, tick: model.tick + 1};
  };
};

let get = ({spring, target, _} as model) =>
  if (isActive(model)) {
    spring.value;
  } else {
    target;
  };

let getTarget = ({target, _}) => target;

let set = (~instant: bool, ~position: float, model) => {
  switch (model.startTime) {
  | None => {
      ...model,
      spring: PhysicalSpring.create(position),
      target: position,
      tick: model.tick + 1,
    }
  | Some(_) when instant => {
      ...model,
      target: position,
      spring: PhysicalSpring.create(position),
    }

  | Some(_) => {...model, target: position, tick: model.tick + 1}
  };
};

// SUB
let sub = model =>
  if (isActive(model) || model.startTime == None) {
    let uniqueId = model.uniqueId ++ "." ++ string_of_int(model.tick);
    Service_Time.Sub.once(~uniqueId, ~delay=Revery.Time.zero, ~msg=(~current) =>
      Tick(current)
    );
  } else {
    Isolinear.Sub.none;
  };
