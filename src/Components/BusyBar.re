open Revery;
open Revery.UI;
open Oni_Core;

module Colors = Feature_Theme.Colors;
module Styles = {
  open Style;
  let progressBarTrack = [height(2), overflow(`Hidden)];

  let progressBarIndicator = (~width, ~offset, ~theme, ~opacity) => [
    height(2),
    Style.width(width),
    transform(Transform.[TranslateX(offset)]),
    backgroundColor(
      Colors.Oni.normalModeBackground.from(theme)
      |> Color.multiplyAlpha(opacity),
    ),
  ];
};

module Animations = {
  open Animation;
  let motion =
    animate(Time.ms(1500))
    |> ease(Easing.easeInOut)
    |> repeat
    |> tween(0., 1.);
};

let%component make = (~visible: bool, ~theme, ()) => {
  let%hook opacity = Hooks.transition(visible ? 1.0 : 0.0);
  let%hook (time, _, _) =
    Hooks.animation(~active=opacity >= 0.2, Animations.motion);

  let%hook (width, setWidth) = Hooks.state(0);

  let indicator = () => {
    let indicatorWidth = 100;
    let trackWidth = float(width + indicatorWidth);
    let offset = trackWidth *. Easing.easeInOut(time) -. float(width);

    <View
      style={Styles.progressBarIndicator(~width, ~offset, ~theme, ~opacity)}
    />;
  };

  <View
    onDimensionsChanged={({width, _}) => setWidth(_ => width)}
    style=Styles.progressBarTrack>
    <indicator />
  </View>;
};
