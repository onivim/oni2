/*
 * SneakView.re
 *
 * View for Sneaks
 */

open Revery;
open Revery.UI;
open Revery.Math;

module Core = Oni_Core;
open Oni_Model;

module Colors = Feature_Theme.Colors;

module Constants = {
  let size = 25;
};

module Styles = {
  let containerStyle =
    Style.[
      backgroundColor(Color.rgba(0.1, 0.1, 0.1, 0.25)),
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
    ];

  let sneakItem = (x, y, theme) =>
    Style.[
      backgroundColor(Colors.Oni.Sneak.background.from(theme)),
      position(`Absolute),
      top(y),
      left(x + Constants.size / 2),
      Style.height(Constants.size),
      Style.width(Constants.size),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];

  let textStyle = (theme, font: Core.UiFont.t) =>
    Style.[
      color(Colors.Oni.Sneak.foreground.from(theme)),
      fontFamily(font.fontFile),
      fontSize(12.),
    ];
  let highlightStyle = (theme, font: Core.UiFont.t) =>
    Style.[
      backgroundColor(Colors.Oni.Sneak.background.from(theme)),
      color(Colors.Oni.Sneak.highlight.from(theme)),
      fontFamily(font.fontFile),
      fontSize(12.),
    ];
};

let make = (~model, ~theme, ~font, ()) => {
  let makeSneak = (bbox, text) => {
    let (x, y, _width, _height) = BoundingBox2d.getBounds(bbox);

    let (highlightText, remainingText) = Sneak.getTextHighlight(text, model);
    <View style={Styles.sneakItem(int_of_float(x), int_of_float(y), theme)}>
      <Text style={Styles.highlightStyle(theme, font)} text=highlightText />
      <Text style={Styles.textStyle(theme, font)} text=remainingText />
    </View>;
  };

  let sneaks = Sneak.getFiltered(model);
  let sneakViews =
    List.map(
      (Sneak.{boundingBox, id, _}) => makeSneak(boundingBox, id),
      sneaks,
    )
    |> React.listToElement;

  let isActive = Sneak.isActive(model);
  isActive
    ? <View style=Styles.containerStyle> sneakViews </View> : React.empty;
};
