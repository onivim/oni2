/*
 * SneakView.re
 *
 * View for Sneaks
 */

open Revery.UI;
open Revery.Math;

module Core = Oni_Core;
open Oni_Model;

module Colors = Feature_Theme.Colors;

module Constants = {
  let size = 25;
};

module Styles = {
  open Style;

  let backdrop = theme => [
    backgroundColor(Colors.Oni.Modal.backdrop.from(theme)),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];

  let item = (x, y, theme) => [
    backgroundColor(Colors.Oni.Sneak.background.from(theme)),
    position(`Absolute),
    boxShadow(
      ~xOffset=3.,
      ~yOffset=3.,
      ~blurRadius=5.,
      ~spreadRadius=0.,
      ~color=Revery.Color.rgba(0., 0., 0., 0.2),
    ),
    top(y),
    left(x + Constants.size / 2),
    Style.height(Constants.size),
    Style.width(Constants.size),
    flexDirection(`Row),
    justifyContent(`Center),
    alignItems(`Center),
  ];

  let text = (theme, font: Core.UiFont.t) => [
    color(Colors.Oni.Sneak.foreground.from(theme)),
    fontFamily(font.fontFile),
    fontSize(12.),
  ];
  let highlight = (theme, font: Core.UiFont.t) => [
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
    <View style={Styles.item(int_of_float(x), int_of_float(y), theme)}>
      <Text style={Styles.highlight(theme, font)} text=highlightText />
      <Text style={Styles.text(theme, font)} text=remainingText />
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
    ? <View style={Styles.backdrop(theme)}> sneakViews </View> : React.empty;
};
