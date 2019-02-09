open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("commandline");

let cmdFontSize = 20;
let cmdFontColor = Colors.white;

let cmdTextStyles =
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    marginLeft(10),
    fontSize(cmdFontSize),
    color(cmdFontColor),
  ];
/*
   TODO: Flow text around a "cursor"
 */

let isZeroIndex = num => num - 1 <= 0;

let safeStrSub = (str, strStart, strEnd) =>
  switch (String.sub(str, strStart, strEnd)) {
  | v => v
  | exception (Invalid_argument(_)) =>
    print_endline(
      "Error getting substring from "
      ++ str
      ++ " starting at "
      ++ string_of_int(strStart)
      ++ " ending at "
      ++ string_of_int(strEnd),
    );
    "";
  };

let getStringParts = (index, str) => {
  switch (String.length(str), index) {
  | (0, _) => ("", "")
  | (_, i) when isZeroIndex(i) => (str, "")
  | (l, _) when isZeroIndex(l) => (str, "")
  | (l, i) when l == i => (str, "")
  | (len, idx) =>
    let strBeginning = safeStrSub(str, 0, idx - 1);
    let strEnd = safeStrSub(str, idx - 1, len - 1);
    (strBeginning, strEnd);
  };
};

let make = (~command: Types.Commandline.t, ~theme: Theme.t) => {
  component((_slots: React.Hooks.empty) => {
    let (startStr, endStr) =
      getStringParts(command.position, command.content);
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
      : React.listToElement([]);
  });
};

let createElement = (~command, ~theme, ~children as _, ()) => {
  React.element(make(~command, ~theme));
};
