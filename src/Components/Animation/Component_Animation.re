module Spring = Spring;

open Revery;
open Revery.UI;

type t('value) = {
  animation: Revery.UI.Animation.t('value),
  startTime: option(Revery.Time.t),
};

let make = animation => {animation, startTime: None};

type outmsg =
| Nothing
| Completed;

type msg =
  | Tick({totalTime: Revery.Time.t});

let update = (_msg, model) => (model, Nothing);
let isActive = _model => false;
let get = ({animation, startTime}) => {
  Animation.valueAt(Time.zero, animation);
};

let sub = _model => Isolinear.Sub.none;
