module Spring: {
  type t;

  type msg;

  let make:
    (~restThreshold: float=?, ~options: Revery.UI.Spring.Options.t=?, float) =>
    t;

  let update: (msg, t) => t;
  let sub: t => Isolinear.Sub.t(msg);

  let isActive: t => bool;

  let set: (~instant: bool, ~position: float, t) => t;
  let get: t => float;
  let getTarget: t => float;
};

type t('value);
type msg;

let get: t('value) => 'value;
let isComplete: t(_) => bool;
let make: Revery.UI.Animation.t('value) => t('value);
let update: (msg, t('value)) => t('value);
let sub: t(_) => Isolinear.Sub.t(msg);
