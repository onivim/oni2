type model = unit;

let initial = ();

[@deriving show]
type msg =
  | Noop;

type outmsg =
  | Nothing
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg));

module Msg = {
  let exthost = _msg => Noop;
};

let update = (_msg, model) => (model, Nothing);
