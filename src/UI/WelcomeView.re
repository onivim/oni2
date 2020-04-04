/*
 * WelcomeView.re
 *
 * Component for the 'welcome' experience
 */

open Revery.UI;
open Oni_Model;
open Oni_Core;

module Model = Oni_Model;

module Styles = {
  let container = (~theme: Theme.t) =>
    Style.[
      backgroundColor(theme.editorBackground),
      color(theme.foreground),
      flexGrow(1),
      flexDirection(`Column),
      justifyContent(`Center),
      alignItems(`Center),
      overflow(`Hidden),
    ];

  let titleText = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(20.),
      color(theme.foreground),
      marginTop(-40),
    ];

  let versionText = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(12.),
      color(theme.foreground),
      marginTop(0),
    ];

  let commandText = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(12.),
      color(theme.foreground),
    ];

  let header =
    Style.[
      flexGrow(1),
      flexDirection(`Column),
      justifyContent(`Center),
      alignItems(`Center),
      margin(0),
    ];

  let controls =
    Style.[
      flexGrow(0),
      flexDirection(`Column),
      justifyContent(`Center),
      alignItems(`Center),
      marginTop(32),
      marginBottom(128),
    ];
};

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

    let commandText = (~theme: Theme.t, ~fontFile, ~fontSize) => [
      fontFamily(fontFile),
      Style.fontSize(fontSize),
      color(theme.foreground),
    ];

    let spacer = Style.[flexGrow(1)];
  };

  let make = (~name: string, ~shortcut: string, ~state: State.t, ()) => {
    let {theme, editorFont, uiFont, _}: State.t = state;

    <View style=Styles.container>
      <Text
        style={Styles.commandText(
          ~theme,
          ~fontFile=uiFont.fontFile,
          ~fontSize=14.,
        )}
        text=name
      />
      <View style=Styles.spacer />
      <Text
        style={Styles.commandText(
          ~theme,
          ~fontFile=editorFont.fontFile,
          ~fontSize=11.,
        )}
        text=shortcut
      />
    </View>;
  };
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(250))
    |> ease(Easing.ease)
    |> tween(0., 1.)
    |> delay(Revery.Time.milliseconds(150))
  );

let%component make = (~state: State.t, ()) => {
  let theme = state.theme;

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
          style={Styles.titleText(~theme, ~font=state.uiFont)}
          text="Modal Editing from the Future"
        />
        <Text
          style={Styles.versionText(~theme, ~font=state.uiFont)}
          text={Printf.sprintf("Version %s", Oni_Core.BuildInfo.version)}
        />
      </View>
      <View style=Styles.controls>
        <KeyBindingView name="Quick open" shortcut="Cmd + P" state />
        <KeyBindingView
          name="Command palette"
          shortcut="Cmd + Shift + P"
          state
        />
        <KeyBindingView name="Vim command" shortcut=":" state />
        <KeyBindingView name="Sneak" shortcut="Ctrl + G" state />
        <KeyBindingView name="Terminal" shortcut=":term" state />
      </View>
    </Opacity>
  </View>;
};
