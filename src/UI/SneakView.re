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

let bgc = Color.rgba(0.1, 0.1, 0.1, 0.25);

module Styles = {
  let containerStyle =
    Style.[
      backgroundColor(bgc),
      position(`Absolute),
      top(0),
      left(0),
      right(0),
      bottom(0),
    ];
};

module Constants = {
  let size = 25;
};

let make = (~state: State.t, ()) => {
  let {theme, _}: State.t = state;
  let makeSneak = (bbox, text) => {
    open Style;
    let (x, y, _width, _height) = BoundingBox2d.getBounds(bbox);
    let sneakStyle = [
      backgroundColor(theme.sneakBackground),
      position(`Absolute),
      top(int_of_float(y) - Constants.size / 2),
      left(int_of_float(x) + Constants.size / 2),
      Style.height(Constants.size),
      Style.width(Constants.size),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];

    let textStyle = [
      backgroundColor(theme.sneakBackground),
      color(theme.sneakForeground),
      fontFamily(state.uiFont.fontFile),
      fontSize(12),
    ];
    let highlightStyle = [
      backgroundColor(theme.sneakBackground),
      color(theme.sneakHighlight),
      fontFamily(state.uiFont.fontFile),
      fontSize(12),
    ];
    let (highlightText, remainingText) =
      Sneak.getTextHighlight(text, state.sneak);
    <View style=sneakStyle>
      <Text style=highlightStyle text=highlightText />
      <Text style=textStyle text=remainingText />
    </View>;
  };

  let sneaks = Sneak.getFiltered(state.sneak);
  let sneakViews =
    List.map(
      (Sneak.{boundingBox, id, _}) => makeSneak(boundingBox, id),
      sneaks,
    )
    |> React.listToElement;

  let isActive = Sneak.isActive(state.sneak);
  isActive
    ? <View style=Styles.containerStyle> sneakViews </View> : React.empty;
};
