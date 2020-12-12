open Revery;
open Revery.UI;

module Colors = Feature_Theme.Colors;

let make = (~children, ~theme, ~config, ()) => {
  let useBoxShadow =
    Feature_Configuration.GlobalConfiguration.shadows.get(config);

  if (useBoxShadow) {
    let color = Color.rgba(0., 0., 0., 0.75);
    <View
      style=[
        Style.backgroundColor(color),
        Style.boxShadow(
          ~xOffset=4.,
          ~yOffset=4.,
          ~blurRadius=12.,
          ~spreadRadius=0.,
          ~color,
        ),
      ]>
      ...children
    </View>;
  } else {
    <View
      style=Style.[
        border(~color=Colors.Editor.background.from(theme), ~width=1),
      ]>
      ...children
    </View>;
  };
};
