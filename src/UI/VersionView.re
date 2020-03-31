/*
 * VersionView.re
 *
 * Component for the 'version' buffer renderer
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
      backgroundColor(Revery.Colors.red),
    ];

  let titleText = (~theme: Theme.t, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(14.),
      color(theme.foreground),
      marginTop(0),
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
};

module VersionView = {
  module Styles = {
    open Style;

    let container = [
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      height(16),
      maxWidth(500),
      minWidth(150),
      backgroundColor(Revery.Colors.blue),
    ];

    let versionText = (~theme: Theme.t, ~fontFile, ~fontSize) => [
      fontFamily(fontFile),
      Style.fontSize(fontSize),
      color(theme.foreground),
    ];

    let spacer = Style.[flexGrow(1)];
  };

  let make = (~name: string, ~version: string, ~state: State.t, ()) => {
    let {theme, editorFont, uiFont, _}: State.t = state;

    <View style=Styles.container>
        <Text
          style={Styles.versionText(
            ~theme,
            ~fontFile=uiFont.fontFile,
            ~fontSize=12.,
          )}
          text=name
        />
      <View style=Styles.spacer />
      <View style=Styles.spacer>
      <Text
        style={Styles.versionText(
          ~theme,
          ~fontFile=editorFont.fontFile,
          ~fontSize=11.,
        )}
        text=version
      />
      </View>
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

let osString = Revery.Environment.os
|> fun
| Windows => "Windows"
| Mac => "OSX"
| _ => "Linux";

let sdlVersionToString = ({major, minor, patch}: Sdl2.Version.t) => {
  Printf.sprintf("%d.%d.%d", major, minor, patch);
};

let%component make = (~state: State.t, ()) => {
  let theme = state.theme;

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let version = VersionView.make(~state);

  <View style={Styles.container(~theme)}>
        <version name="Version:" version=Oni_Core.BuildInfo.version />
        <version name="Commit:" version=Oni_Core.BuildInfo.commitId />
        // spacer
        <version name="OCaml:" version=Sys.ocaml_version />
        // spacer
        <version name="SDL Compiled:" version={Sdl2.Version.getCompiled() |> sdlVersionToString} />
        <version name="SDL Linked:" version={Sdl2.Version.getLinked() |> sdlVersionToString} />
        // spacer
        <version name="GL Version:" version={Sdl2.Gl.getString(Sdl2.Gl.Version)} />
        <version name="GL Vendor:" version={Sdl2.Gl.getString(Sdl2.Gl.Vendor)} />
        <version name="GL Renderer:" version={Sdl2.Gl.getString(Sdl2.Gl.Renderer)} />
        <version name="GL GLSL:" version={Sdl2.Gl.getString(Sdl2.Gl.ShadingLanguageVersion)} />
  </View>;
};
