type model = unit;

let initial = ();

type outmsg =
  | Nothing;

type msg =
  | Noop;

module Msg = {
  let exthost = _msg => Noop;
};

let update = (_msg, model) => (model, Nothing);
