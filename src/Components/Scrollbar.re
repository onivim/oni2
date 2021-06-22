open Revery.UI;

module Colors = Feature_Theme.Colors;

module Constants = {
  let scrollBarThickness = 6;
  let minimumThumbSize = 4;
};

module Styles = {
  open Revery.UI.Style;
  let slider = [
    position(`Absolute),
    right(0),
    top(0),
    bottom(0),
    width(Constants.scrollBarThickness),
    overflow(`Hidden),
  ];
};

let make =
    (
      ~config,
      ~pixelHeight,
      ~viewportHeight,
      ~totalHeight,
      ~scrollY,
      ~onScroll,
      ~theme,
      (),
    ) => {
  // TODO: Bring back scroll-speed
  ignore(config);

  let maxHeight = totalHeight -. viewportHeight;
  let thumbHeight =
    pixelHeight
    *. viewportHeight
    /. max(1., maxHeight)
    |> max(float(Constants.minimumThumbSize));
  let isVisible = maxHeight > 0.;

  if (isVisible) {
    <View style=Styles.slider>
      <Revery.UI.Components.Slider
        onValueChanged=onScroll
        minimumValue=0.
        maximumValue=maxHeight
        sliderLength={int_of_float(pixelHeight)}
        thumbLength={int_of_float(thumbHeight)}
        value=scrollY
        trackThickness=Constants.scrollBarThickness
        thumbThickness=Constants.scrollBarThickness
        minimumTrackColor=Revery.Colors.transparentBlack
        maximumTrackColor=Revery.Colors.transparentBlack
        thumbColor={Colors.ScrollbarSlider.activeBackground.from(theme)}
        vertical=true
      />
    </View>;
  } else {
    React.empty;
  };
};
