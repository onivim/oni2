/*
 * Component_Output.re
 *
 * A simple component for showing terminal-style output
 */

[@deriving show]
type msg = unit;

[@deriving show]
type model = unit;

let initial = ();

let set = (_contents, model) => model;

type outmsg =
  | Nothing;

let update = (_msg, model) => (model, Nothing);

let keyPress = (_key, model) => model;

module Contributions = {
  let commands = [];
  let keybindings = [];
};

module View = {
  open Revery;
  open Revery.UI;

  let make =
      (
        ~isActive as _,
        ~editorFont as _,
        ~theme as _,
        ~model as _,
        ~dispatch as _,
        (),
      ) => {
    <Text text="todo" />;
  };
};

module Sub = {
  let sub = _model => Isolinear.Sub.none;
};
