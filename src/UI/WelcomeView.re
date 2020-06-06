/*
 * WelcomeView.re
 *
 * Component for the 'welcome' experience
 */

open Revery.UI;
open Oni_Core;

module Model = Oni_Model;

module Colors = Feature_Theme.Colors;

module KeyBindingView = {
  module Styles = {
    open Style;

    let container = [
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      height(25),
      maxWidth(300),
      minWidth(150),
    ];

    let command = (~theme) => [color(Colors.foreground.from(theme))];

    let spacer = Style.[flexGrow(1)];
  };

  let make =
      (
        ~name: string,
        ~shortcut: string,
        ~theme,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        (),
      ) => {
    <View style=Styles.container>
      <Text
        style={Styles.command(~theme)}
        fontFamily={uiFont.normal}
        fontSize=14.
        text=name
      />
      <View style=Styles.spacer />
      <Text
        style={Styles.command(~theme)}
        fontFamily={editorFont.fontFamily}
        fontSize=11.
        text=shortcut
      />
    </View>;
  };
};

module Styles = {
  open Style;

  let container = (~theme) => [
    backgroundColor(Colors.Editor.background.from(theme)),
    color(Colors.foreground.from(theme)),
    flexGrow(1),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
    overflow(`Hidden),
  ];

  let title = (~theme) => [
    color(Colors.foreground.from(theme)),
    marginTop(-40),
  ];

  let version = (~theme) => [
    color(Colors.foreground.from(theme)),
    marginTop(0),
  ];

  let header = [
    flexGrow(1),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
    margin(0),
  ];

  let controls = [
    flexGrow(0),
    flexDirection(`Column),
    justifyContent(`Center),
    alignItems(`Center),
    marginTop(32),
    marginBottom(128),
  ];
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(250))
    |> ease(Easing.ease)
    |> tween(0., 1.)
    |> delay(Revery.Time.milliseconds(150))
  );

let%component make = (~theme, ~uiFont: UiFont.t, ~editorFont, ()) => {
  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  <View style={Styles.container(~theme)}>
    <Opacity opacity=transition>
      <View style=Styles.header>
        <Image
          src="./title-logo.png"
          width=456
          height=250
          opacity=transition
        />
        <Text
          style={Styles.title(~theme)}
          fontFamily={uiFont.normal}
          fontSize=20.
          text="Modal Editing from the Future"
        />
        <Text
          style={Styles.version(~theme)}
          fontFamily={uiFont.normal}
          fontSize=12.
          text={Printf.sprintf("Version %s", Oni_Core.BuildInfo.version)}
        />
      </View>
      <View style=Styles.controls>
        <KeyBindingView
          name="Quick open"
          shortcut="Cmd + P"
          theme
          uiFont
          editorFont
        />
        <KeyBindingView
          name="Command palette"
          shortcut="Cmd + Shift + P"
          theme
          uiFont
          editorFont
        />
        <KeyBindingView
          name="Vim command"
          shortcut=":"
          theme
          uiFont
          editorFont
        />
        <KeyBindingView
          name="Sneak"
          shortcut="Ctrl + G"
          theme
          uiFont
          editorFont
        />
        <KeyBindingView
          name="Terminal"
          shortcut=":term"
          theme
          uiFont
          editorFont
        />
      </View>
    </Opacity>
  </View>;
};
