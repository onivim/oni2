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
  let text = (~color) => [textWrap(TextWrapping.Wrap), Style.color(color)];
};

let textToElement = (~color, ~font: UiFont.t, ~text) => {
  <Text
    style={Styles.text(~color)}
    fontSize=11.
    fontFamily={font.family}
    text
  />;
};

let iconNameToCharacter = name => {
  let iconCode = name |> Codicon.stringToGlyph;

  if (iconCode == Codicon.Constants.none) {
    None;
  } else {
    Some(iconCode);
  };
};

let iconToElement = (~color, icon) => {
  <View style=Style.[margin(4)]> <Codicon icon color /> </View>;
};

let make = (~font, ~color, ~label: Exthost.Label.t, ()) => {
  Exthost.Label.(
    label
    |> segments
    |> List.map(
         fun
         | Text(text) => textToElement(~color, ~font, ~text)
         | Icon(iconName) =>
           iconName
           |> iconNameToCharacter
           |> Option.map(iconToElement(~color))
           |> Option.value(~default=React.empty),
       )
    |> React.listToElement
  );
};
