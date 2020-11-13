open Revery;
open Revery.UI;

// MODEL

type msg =
| Tick;

module UniqueId = UniqueId.Make({});

type t = {
    options: Spring.Options.t,
    spring: Spring.t,
    target: float,
    restThreshold: float,
    uniqueId: string,
};

let make = (~restThreshold=1.0, ~options=Spring.Options.default, position) => {
    options,
    target: position,
    spring: Spring.create(position, Time.zero),
    restThreshold,
    uniqueId: "Service_Animation.spring" ++ string_of_int(UniqueId.getUniqueId()),
};

// UPDATE

let update = (~time, msg, model) => {
    switch (msg) {
    | Tick =>
        let spring = Spring.tick(model.target, model.spring, model.options, time);
        {...model, spring}
    }
}

let get = ({spring, _}) => Spring.(spring.value);

let set = (~position: float, model) => {
    ({...model, target: position})
};

let isActive = ({spring, restThreshold, _}) => {
    !Spring.isAtRest(~restThreshold, spring)
};

// SUB
let sub = (model) => {
    if (isActive(model)) {
        Service_Time.Sub.once(~uniqueId=model.uniqueId, ~delay=Revery.Time.zero, ~msg=Tick);
    } else {
        Isolinear.Sub.none
    }
};

