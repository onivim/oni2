/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery.UI;
open Oni_Core.CamomileBundled.Camomile;
module ZedBundled = Oni_Core.ZedBundled;

let codeToIcon = icon => ZedBundled.singleton(UChar.of_int(icon));

let make =
    (
      ~icon,
      ~fontFamily="FontAwesome5FreeSolid.otf",
      ~fontSize=15,
      ~backgroundColor,
      ~color,
      ~margin=0,
      (),
    ) => {
  let (fontFamily_, fontSize_, backgroundColor_, color_, margin_) = (
    fontFamily,
    fontSize,
    backgroundColor,
    color,
    margin,
  );

  <Text
    text={codeToIcon(icon)}
    style=Style.[
      fontFamily(fontFamily_),
      fontSize(fontSize_),
      color(color_),
      backgroundColor(backgroundColor_),
      margin(margin_),
    ]
  />;
};
