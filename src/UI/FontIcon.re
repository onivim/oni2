/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery.UI;
open Oni_Core.CamomileBundled.Camomile;
module ZedBundled = Oni_Core.ZedBundled;

let component = React.component("FontIcon");

let codeToIcon = icon => ZedBundled.singleton(UChar.of_int(icon));

let createElement =
    (
      ~icon,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=15,
      ~backgroundColor,
      ~color,
      ~children as _,
      (),
    ) =>
  component(hooks => {
    let (fontFamily_, fontSize_, backgroundColor_, color_) = (
      fontFamily,
      fontSize,
      backgroundColor,
      color,
    );

    (
      hooks,
      <Text
        text={codeToIcon(icon)}
        style=Style.[
          fontFamily(fontFamily_),
          fontSize(fontSize_),
          color(color_),
          backgroundColor(backgroundColor_),
        ]
      />,
    );
  });
