open Revery.UI;
open Revery.Core;
/* open Revery.UI.Components; */
open Oni_Core;

let component = React.component("commandline");

let make = (~command: Types.Commandline.t) => {
  component((_slots: React.Hooks.empty) =>
    command.show
      ? <View
          style=Style.[
            width(200),
            height(300),
            position(`Absolute),
            top(30),
            left(200),
            backgroundColor(Colors.white),
          ]>
          <Text
            style=Style.[
              fontFamily("FiraCode-Regular.ttf"),
              fontSize(25),
              color(Colors.black),
            ]
            text={command.content}
          />
        </View>
      : <View />
  );
};

let createElement = (~command, ~children as _, ()) => {
  React.element(make(~command));
};
