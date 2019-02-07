open Revery.UI;
open Revery.Core;
/* open Revery.UI.Components; */
open Oni_Core;

let component = React.component("commandline");

let make = (~command: Types.Commandline.t, ~theme: Theme.t) => {
  component((_slots: React.Hooks.empty) =>
    command.show
      ? <View
          style=Style.[
            position(`Relative),
            top(0),
            right(0),
            left(0),
            bottom(0),
          ]>
          <View
            style=Style.[
              width(400),
              height(100),
              top(30),
              left(200),
              backgroundColor(theme.editorBackground),
              boxShadow(
                ~xOffset=-10.,
                ~yOffset=20.,
                ~blurRadius=10.,
                ~spreadRadius=2.,
                ~color=Color.rgba(0., 0., 0., 0.5),
              ),
            ]>
            <Text
              style=Style.[
                fontFamily("FiraCode-Regular.ttf"),
                marginTop(10),
                marginLeft(10),
                fontSize(20),
                color(Colors.white),
              ]
              text={command.firstC ++ command.content}
            />
          </View>
        </View>
      : <View />
  );
};

let createElement = (~command, ~theme, ~children as _, ()) => {
  React.element(make(~command, ~theme));
};
