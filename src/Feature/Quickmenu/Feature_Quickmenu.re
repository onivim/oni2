module Schema = {
  type menu('outmsg) = unit;

  let menu =
      (~onItemFocused=?, ~onItemSelected=?, ~onCancelled=?, ~initialItems) =>
    ();

  let map = (f, model) => model;
};

// MODEL

[@deriving show]
type msg =
  | KeyPressed(string)
  | Pasted(string);

module Msg = {
  let keyPressed = key => KeyPressed(key);
  let pasted = key => Pasted(key);
};

type model('outmsg) = {menus: list(Schema.menu('outmsg))};

let initial = {menus: []};

//let isMenuOpen = ({menus}) => menus != [];
let isMenuOpen = _ => true;

let show = (~menu, model) => {menus: [menu, ...model.menus]};

// UPDATE

type outmsg('action) =
  | Action('action)
  | Nothing;

let update = (msg, model) => {
  prerr_endline("MSG: " ++ show_msg(msg));
  (model, Nothing);
};

// SUBSCRIPTION

let sub = _model => Isolinear.Sub.none;

module View = {
  open Revery.UI;

  let make = (~font, ~theme, ~config, ~model, ~dispatch, ()) => {
    <View />;
  };
};
