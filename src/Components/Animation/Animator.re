module UniqueId =
  Oni_Core.UniqueId.Make({});

type t('model, 'interpolatedModel) = {
  prevInterpolatedModel: 'interpolatedModel,
  nextInterpolatedModel: 'interpolatedModel,
  uniqueId: UniqueId.t,
  spring: Spring.t,
  equalsFn: ('interpolatedModel, 'interpolatedModel) => bool,
  toInterpolatedFn: 'model => 'interpolatedModel,
};

let create = (~equals=(==), ~initial, toInterpolated) => {
  prevInterpolatedModel: initial,
  nextInterpolatedModel: initial,
  uniqueId: UniqueId.create(~friendlyName="Component_Animator"),
  spring: Spring.make(100.),
  toInterpolatedFn: toInterpolated,
  equalsFn: equals,
};

type msg =
  | Spring(Spring.msg);

let get = model => model.nextInterpolatedModel;

let toggleSpring = (~instant, spring) =>
  if (Float.equal(Spring.getTarget(spring), 100.)) {
    Spring.set(~instant, ~position=0., spring);
  } else {
    Spring.set(~instant, ~position=100., spring);
  };

let set = (~instant, newModel, model) => {
  let nextInterpolatedModel = model.toInterpolatedFn(newModel);

  if (!model.equalsFn(model.nextInterpolatedModel, nextInterpolatedModel)) {
    let spring = toggleSpring(~instant, model.spring);
    {
      ...model,
      spring,
      prevInterpolatedModel: model.nextInterpolatedModel,
      nextInterpolatedModel,
    };
  } else {
    model;
  };
};

let sub = ({spring, _}) => {
  Spring.sub(spring) |> Isolinear.Sub.map(msg => Spring(msg));
};

let update = (msg, model) => {
  switch (msg) {
  | Spring(springMsg) =>
    let spring = Spring.update(springMsg, model.spring);
    {...model, spring};
  };
};

let render = (renderFunc, model) => {
  let interp =
    (Spring.getTarget(model.spring) -. Spring.get(model.spring)) /. 100.;

  let interp' = 1.0 -. Float.abs(interp);

  renderFunc(
    ~prev=model.prevInterpolatedModel,
    ~next=model.nextInterpolatedModel,
    interp',
  );
};
