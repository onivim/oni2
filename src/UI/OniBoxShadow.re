open Revery;
open Revery.UI;
open Oni_Core;

let make = (~children, ~theme: Theme.t, ~configuration: Configuration.t, ()) => {
  let useBoxShadow = Configuration.getValue(c => c.uiShadows, configuration);

  if (useBoxShadow) {
    <BoxShadow
      boxShadow={Style.BoxShadow.make(
        ~xOffset=-11.,
        ~yOffset=-11.,
        ~blurRadius=25.,
        ~spreadRadius=0.,
        ~color=Color.rgba(0., 0., 0., 0.2),
        (),
      )}>
      ...children
    </BoxShadow>;
  } else {
    <View style=Style.[border(~color=theme.background, ~width=1)]>
      ...children
    </View>;
  };
};
