open Oni_Core;

open Revery.UI;
open Revery.UI.Components;

type action('msg) = {
  label: string,
  msg: 'msg,
};

module Styles = {
  open Style;

  let container = (~theme: Theme.t) => [backgroundColor(theme.background)];

  let message = [padding(10)];

  let messageText = (~theme: Theme.t, ~font: UiFont.t) => [
    fontFamily(font.fontFile),
    color(theme.foreground),
    backgroundColor(theme.editorBackground),
    fontSize(14),
  ];

  let actions = [flexDirection(`Row)];

  let buttonOuter = (~isHovered, ~theme: Theme.t) => [
    isHovered
      ? backgroundColor(theme.menuSelectionBackground)
      : backgroundColor(theme.editorBackground),
    flexGrow(1),
  ];

  let buttonInner = [padding(10)];

  let buttonText = (~isHovered, ~theme: Theme.t, ~font: UiFont.t) => [
    fontFamily(font.fontFile),
    color(theme.foreground),
    isHovered
      ? backgroundColor(theme.menuSelectionBackground)
      : backgroundColor(theme.editorBackground),
    fontSize(14),
    alignSelf(`Center),
  ];
};

let%component button = (~text, ~onClick, ~theme, ~font, ()) => {
  let%hook (isHovered, setHovered) = Hooks.state(false);

  <Clickable onClick style={Styles.buttonOuter(~theme, ~isHovered)}>
    <View
      onMouseOver={_ => setHovered(_ => true)}
      onMouseOut={_ => setHovered(_ => false)}
      style=Styles.buttonInner>
      <Text style={Styles.buttonText(~isHovered, ~theme, ~font)} text />
    </View>
  </Clickable>;
};

let make = (~message, ~theme, ~font, ~actions, ~onAction, ()) =>
  <View style={Styles.container(~theme)}>
    <View style=Styles.message>
      <Text style={Styles.messageText(~theme, ~font)} text=message />
    </View>
    <View style=Styles.actions>
      {actions
       |> List.map(action =>
            <button
              text={action.label}
              onClick={() => onAction(action.msg)}
              theme
              font
            />
          )
       |> React.listToElement}
    </View>
  </View>;
