open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("commandline");

let cmdFontSize = 20;
let cmdFontColor = Colors.white;

let cmdTextStyles =
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    marginTop(10),
    marginLeft(10),
    fontSize(cmdFontSize),
    color(cmdFontColor),
  ];
/*
   TODO: Flow text around a "cursor"
 */

let isZeroIndex = num => num - 1 <= 0;

let getStringParts = (index, str) => {
  switch (String.length(str), index) {
  | (0, _) => ("", "")
  | (_, i) when isZeroIndex(i) => (str, "")
  | (l, _) when isZeroIndex(l) => (str, "")
  | (len, idx) =>
    let strBeginning = String.sub(str, 0, idx - 1);
    let strEnd = String.sub(str, idx - 1, len);
    print_endline("strBeginning =========================: " ++ strBeginning);
    print_endline("strEnd: " ++ strEnd);
    (strBeginning, strEnd);
  };
};

let make = (~command: Types.Commandline.t, ~theme: Theme.t) => {
  component((_slots: React.Hooks.empty) => {
    let (startStr, endStr) =
      getStringParts(command.position, command.content);
    print_endline(
      "string_of_int(command.index),
    : "
      ++ string_of_int(command.position),
    );
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
              flexDirection(`Row),
              alignItems(`Center),
              boxShadow(
                ~xOffset=-15.,
                ~yOffset=5.,
                ~blurRadius=30.,
                ~spreadRadius=5.,
                ~color=Color.rgba(0., 0., 0., 0.2),
              ),
            ]>
            <Text style=cmdTextStyles text={command.firstC ++ startStr} />
            <View
              style=Style.[
                width(3),
                height(cmdFontSize),
                backgroundColor(cmdFontColor),
              ]
            />
            <Text style=cmdTextStyles text=endStr />
          </View>
        </View>
      : <View />;
  });
};

let createElement = (~command, ~theme, ~children as _, ()) => {
  React.element(make(~command, ~theme));
};
