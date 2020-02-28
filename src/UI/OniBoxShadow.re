open Revery;
open Revery.UI;
open Oni_Core;

let make = (~children, ~theme: Theme.t, ~configuration: Configuration.t, ()) => {
  let useBoxShadow = Configuration.getValue(c => c.uiShadows, configuration);

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
    <View style=Style.[border(~color=theme.editorBackground, ~width=1)]>
      ...children
    </View>;
  };
};
