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

module Constants = {
  let size = 25;
};

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

  let sneakItem = (x, y, theme: Core.Theme.t) =>
    Style.[
      backgroundColor(theme.sneakBackground),
      position(`Absolute),
      top(y),
      left(x + Constants.size / 2),
      Style.height(Constants.size),
      Style.width(Constants.size),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];

  let textStyle = (theme: Core.Theme.t, uiFont: Core.UiFont.t) =>
    Style.[
      backgroundColor(theme.sneakBackground),
      color(theme.sneakForeground),
      fontFamily(uiFont.fontFile),
      fontSize(12.),
    ];
  let highlightStyle = (theme: Core.Theme.t, uiFont: Core.UiFont.t) =>
    Style.[
      backgroundColor(theme.sneakBackground),
      color(theme.sneakHighlight),
      fontFamily(uiFont.fontFile),
      fontSize(12.),
    ];
};

let make = (~state: State.t, ()) => {
  let {theme, uiFont, _}: State.t = state;
  let makeSneak = (bbox, text) => {
    let (x, y, _width, _height) = BoundingBox2d.getBounds(bbox);

    let (highlightText, remainingText) =
      Sneak.getTextHighlight(text, state.sneak);
    <View style={Styles.sneakItem(int_of_float(x), int_of_float(y), theme)}>
      <Text style={Styles.highlightStyle(theme, uiFont)} text=highlightText />
      <Text style={Styles.textStyle(theme, uiFont)} text=remainingText />
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
