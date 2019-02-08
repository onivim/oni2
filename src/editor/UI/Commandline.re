open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("commandline");

let make = (~command: Types.Commandline.t, ~theme: Theme.t) => {
  component((_slots: React.Hooks.empty) =>
    command.show
      ? <View
          style=Style.[
            position(`Absolute),
            top(0),
            right(0),
            left(0),
            bottom(0),
            alignItems(`Center),
          ]>
          <View
            style=Style.[
              width(400),
              height(40),
              top(50),
              backgroundColor(theme.editorBackground),
              boxShadow(
                ~xOffset=-15.,
                ~yOffset=5.,
                ~blurRadius=30.,
                ~spreadRadius=5.,
                ~color=Color.rgba(0., 0., 0., 0.2),
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
