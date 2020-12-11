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
        ~theme: Oni_Core.ColorTheme.Colors.t,
        ~model,
        ~dispatch as _,
        (),
      ) => {
    let bg = Feature_Theme.Colors.Terminal.background.from(theme);
    let fg = Feature_Theme.Colors.Terminal.foreground.from(theme);
    <View style=Style.[backgroundColor(bg)]>
      <Text
        fontFamily={editorFont.fontFamily}
        fontSize={editorFont.fontSize}
        text={model.contents}
        style=Style.[color(fg)]
      />
    </View>;
  };
};

module Sub = {
  let sub = _model => Isolinear.Sub.none;
};
