/*
 * Component_Output.re
 *
 * A simple component for showing terminal-style output
 */

[@deriving show]
type msg = unit;

[@deriving show]
type model = {contents: string};

let initial = {contents: ""};

let set = (contents, _model) => {contents: contents};

type outmsg =
  | Nothing;

let update = (_msg, model) => (model, Nothing);

let keyPress = (_key, model) => model;

module Contributions = {
  let commands = [];
  let keybindings = [];
};

module View = {
  open Revery.UI;

  let make =
      (
        ~isActive as _,
        ~editorFont: Service_Font.font,
        ~theme as _,
        ~model,
        ~dispatch as _,
        (),
      ) => {
    <Text
      fontFamily={editorFont.fontFamily}
      fontSize={editorFont.fontSize}
      text={model.contents}
    />;
  };
};

module Sub = {
  let sub = _model => Isolinear.Sub.none;
};
