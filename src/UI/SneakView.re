/*
 * SneakView.re
 *
 * View for Sneaks
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

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
      bottom(0)
    ];
};

let make = (~state: State.t, ()) => {

  let isActive = Sneak.isActive(state.sneak);
  isActive ? <View style=Styles.containerStyle>
  </View> : React.empty;
};
