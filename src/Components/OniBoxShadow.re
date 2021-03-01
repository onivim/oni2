open Revery;
open Revery.UI;

module Colors = Feature_Theme.Colors;

let make = (~onMouseDown=?, ~style=[], ~children, ~theme, ~config, ()) => {
  let useBoxShadow =
    Feature_Configuration.GlobalConfiguration.shadows.get(config);

  if (useBoxShadow) {
    let color = Color.rgba(0., 0., 0., 0.75);
    <View
      ?onMouseDown
      style=[
        Style.backgroundColor(color),
        Style.boxShadow(
          ~xOffset=4.,
          ~yOffset=4.,
          ~blurRadius=12.,
          ~spreadRadius=0.,
          ~color,
        ),
        ...style,
      ]>
      ...children
    </View>;
  } else {
    <View
      ?onMouseDown
      style=Style.[
        border(~color=Colors.Editor.background.from(theme), ~width=1),
        ...style,
      ]>
      ...children
    </View>;
  };
};
