/*
 * VersionView.re
 *
 * Component for the 'version' buffer renderer
 */

open Revery.UI;
open Oni_Core;

module Colors = Feature_Theme.Colors;

module Styles = {
  let container = (~theme) =>
    Style.[
      backgroundColor(Colors.Editor.background.from(theme)),
      flexDirection(`Column),
      flexGrow(1),
      justifyContent(`Center),
      alignItems(`Center),
      overflow(`Hidden),
    ];

  let versionText = (~theme, ~font: UiFont.t) =>
    Style.[
      fontFamily(font.fontFile),
      fontSize(12.),
      color(Colors.foreground.from(theme)),
      marginTop(0),
    ];
};

module HeaderView = {
  module Styles = {
    let header = (~uiFont: UiFont.t) =>
      Style.[
        flexGrow(0),
        flexDirection(`Column),
        justifyContent(`Center),
        alignItems(`Center),
        marginTop(16),
        marginBottom(0),
        fontFamily(uiFont.fontFile),
        fontSize(14.),
      ];
  };

  let make = (~text, ~uiFont, ()) => {
    <Text style={Styles.header(~uiFont)} text />;
  };
};

module VersionView = {
  module Styles = {
    open Style;

    let container = [
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
      height(18),
      maxWidth(400),
      minWidth(150),
      flexGrow(0),
      borderBottom(~color=Revery.Color.rgba(1.0, 1.0, 1.0, 0.2), ~width=1),
    ];

    let versionText = (~theme, ~fontFile, ~fontSize) => [
      fontFamily(fontFile),
      Style.fontSize(fontSize),
      color(Colors.foreground.from(theme)),
    ];

    let versionValue = [flexGrow(0), alignItems(`FlexEnd)];

    let spacer = Style.[flexGrow(1)];
  };

  let make =
      (
        ~name: string,
        ~version: string,
        ~theme,
        ~uiFont: UiFont.t,
        ~editorFont: Service_Font.font,
        (),
      ) => {
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
        <View style=Styles.versionValue>
          <Text
            style={Styles.versionText(
              ~theme,
              ~fontFile=editorFont.fontFile,
              ~fontSize=11.,
            )}
            text=version
          />
        </View>
      </View>
    </View>;
  };
};

let osString =
  Revery.Environment.os
  |> (
    fun
    | Windows => "Windows"
    | Mac => "OSX"
    | _ => "Linux"
  );

let sdlVersionToString = ({major, minor, patch}: Sdl2.Version.t) => {
  Printf.sprintf("%d.%d.%d", major, minor, patch);
};

let make = (~theme, ~uiFont, ~editorFont, ()) => {
  let version = VersionView.make(~theme, ~uiFont, ~editorFont);
  let header = HeaderView.make(~uiFont);

  <View style={Styles.container(~theme)}>
    <header text="Onivim 2" />
    <version name="Version" version=Oni_Core.BuildInfo.version />
    <version name="Commit" version=Oni_Core.BuildInfo.commitId />
    // spacer
    <header text="OCaml" />
    <version name="Compiler Version " version=Sys.ocaml_version />
    <header text="libuv" />
    <version name="Version " version={Luv.Version.string()} />
    // spacer
    <header text="SDL" />
    <version
      name="SDL Compiled"
      version={Sdl2.Version.getCompiled() |> sdlVersionToString}
    />
    <version
      name="SDL Linked"
      version={Sdl2.Version.getLinked() |> sdlVersionToString}
    />
    // spacer
    <header text="OpenGL" />
    <version name="Version" version={Sdl2.Gl.getString(Sdl2.Gl.Version)} />
    <version name="Vendor" version={Sdl2.Gl.getString(Sdl2.Gl.Vendor)} />
    <version name="Renderer" version={Sdl2.Gl.getString(Sdl2.Gl.Renderer)} />
    <version
      name="GLSL"
      version={Sdl2.Gl.getString(Sdl2.Gl.ShadingLanguageVersion)}
    />
  </View>;
};
