/*
 * FontIcon.re
 *
 * Helper component for using icon fonts, like FontAwesome
 */

open Revery.UI;
open Oni_Core.CamomileBundled.Camomile;
module ZedBundled = Oni_Core.ZedBundled;

module Styles = {
  open Style;

  let text = (~fontFamily, ~fontSize, ~color, ~backgroundColor, ~margin) => [
    Style.fontFamily(fontFamily),
    Style.fontSize(fontSize),
    Style.color(color),
    Style.backgroundColor(backgroundColor),
    Style.margin(margin),
    textWrap(Revery.TextWrapping.NoWrap),
  ];
};

let codeToIcon = icon => ZedBundled.singleton(UChar.of_int(icon));

let make =
    (
      ~icon,
      ~fontFamily=FontAwesome.fontFamily,
      ~fontSize=15.,
      ~backgroundColor,
      ~color,
      ~margin=0,
      (),
    ) =>
  <Text
    text={codeToIcon(icon)}
    style={Styles.text(
      ~fontFamily,
      ~fontSize,
      ~color,
      ~backgroundColor,
      ~margin,
    )}
  />;
