open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Types;

let component = React.component("Textlink");

let textStyles = (theme: Theme.t, font: UiFont.t) =>
  Style.[
    fontSize(20),
    fontFamily(font.fontFile),
    color(theme.colors.foreground),
  ];

let createElement =
    (
      ~children as _,
      ~text,
      ~style as _=?,
      ~font: UiFont.t,
      ~theme: Theme.t,
      ~onClick=() => (),
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      <Clickable onClick>
        <Text style={textStyles(theme, font)} text />
      </Clickable>,
    )
  );
