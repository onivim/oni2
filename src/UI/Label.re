
/*
 * Label.re
 *
 * Container to help render Exthost.Label.t
 */

open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Components;

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

let iconNameToCharacter = fun
| "alert" => Some(FontAwesome.exclamationTriangle)
| _ => None;


let iconToElement = (~color, icon) => {
  <View style=Style.[margin(4)]>
  <FontIcon icon color />
  </View>
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
  | Icon(iconName) => 
      iconName
      |> iconNameToCharacter
      |> Option.map(iconToElement(~color))
      |> Option.value(~default=React.empty)
  )
  |> React.listToElement;
};
