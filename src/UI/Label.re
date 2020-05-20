
/*
 * Label.re
 *
 * Container to help render Exthost.Label.t
 */

open Revery;
open Revery.UI;
open Oni_Core;

module Styles = {
  open Style;
  let text = (~fontSize=11., ~color, uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    Style.fontSize(fontSize),
    textWrap(TextWrapping.NoWrap),
    Style.color(color),
  ];
};
  
 
let textToElement = (~color, ~font, ~text) => {
  <Text style=Styles.text(~color, font) text />
};
  
let make =
              (
                ~font,
                ~color,
                ~label: Exthost.Label.t,
                (),
              ) => {

  open Exthost.Label;

  label
  |> List.map(fun
  | Text(text) => textToElement(~color, ~font, ~text)
  | Icon(_) => React.empty
  )
  |> React.listToElement;
};
