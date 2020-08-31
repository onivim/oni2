open EditorCoreTypes;
open Revery.UI;

open Oni_Core;

module Log = (val Log.withNamespace("Oni2.UI.EditorSurface"));

module Styles = {
  open Style;

  let overlay = [
    pointerEvents(`Ignore),
    position(`Absolute),
    top(0),
    left(0),
    right(0),
    bottom(0),
  ];
};

module Animations = {
  let fadeIn =
    Revery.UI.Animation.(
      animate(Revery.Time.milliseconds(250))
      |> ease(Easing.easeIn)
      |> tween(0.2, 0.0)
      |> delay(Revery.Time.milliseconds(0))
    );
};

let%component make = (~pixelRanges: list(PixelRange.t), ()) => {
  let ranges = pixelRanges;

  let%hook (opacity, _animationState, _reset) =
    Hooks.animation(Animations.fadeIn, ~active=true);

  let elements =
    ranges
    |> List.map((range: PixelRange.t) => {
         <View
           style=Style.[
             position(`Absolute),
             top(range.start.y |> int_of_float),
             left(range.start.x |> int_of_float),
             width(range.stop.x -. range.start.x |> int_of_float),
             height(range.stop.y -. range.start.y |> int_of_float),
             backgroundColor(Revery.Colors.yellow),
           ]
         />
       })
    |> React.listToElement;

  <View style=Styles.overlay> <Opacity opacity> elements </Opacity> </View>;
};
