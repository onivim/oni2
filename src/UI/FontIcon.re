/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery.UI;
open Oni_Core_Kernel.CamomileBundled.Camomile;
module ZedBundled = Oni_Core_Kernel.ZedBundled;

let codeToIcon = icon => ZedBundled.singleton(UChar.of_int(icon));

let make =
    (
      ~icon,
      ~fontFamily=FontAwesome.fontFamily,
      ~fontSize=15,
      ~backgroundColor,
      ~color,
      ~margin=0,
      (),
    ) =>
  <Text
    text={codeToIcon(icon)}
    style=[
      Style.fontFamily(fontFamily),
      Style.fontSize(fontSize),
      Style.color(color),
      Style.backgroundColor(backgroundColor),
      Style.margin(margin),
      Style.height(fontSize),
      Style.width(fontSize),
      Style.textWrap(Revery.TextWrapping.NoWrap),
    ]
  />;
