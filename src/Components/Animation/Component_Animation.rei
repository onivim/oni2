type msg;

module Spring: {
  type t;

  let make:
    (~restThreshold: float=?, ~options: Revery.UI.Spring.Options.t=?, float) =>
    t;

  let update: (msg, t) => t;
  let sub: t => Isolinear.Sub.t(msg);

  let isActive: t => bool;

  let set: (~instant: bool, ~position: float, t) => t;
  let get: t => float;
  let getTarget: t => float;

  let toDebugString: t => string;
};

module ColorTransition: {
  type t;

  type msg;

  let make:
    (~duration: Revery.Time.t, ~delay: Revery.Time.t, Revery.Color.t) => t;

  let update: (msg, t) => t;
  let sub: t => Isolinear.Sub.t(msg);

  let set: (~instant: bool, ~color: Revery.Color.t, t) => t;
  let get: t => Revery.Color.t;
};

type t('value);

let get: t('value) => 'value;
let isComplete: t(_) => bool;
let isActive: t(_) => bool;
let constant: 'value => t('value);
let make: Revery.UI.Animation.t('value) => t('value);
let update: (msg, t('value)) => t('value);
let sub: t(_) => Isolinear.Sub.t(msg);
let subAny: (~uniqueId: string) => Isolinear.Sub.t(msg);
