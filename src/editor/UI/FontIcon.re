/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery.UI;
open CamomileLibraryDefault.Camomile;

let component = React.component("FontIcon");

let codeToIcon = icon => Zed_utf8.singleton(UChar.of_int(icon));

let createElement =
    (
      ~icon,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=50,
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
          width(50),
          height(50),
        ]
      />,
    );
  });
