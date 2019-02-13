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

let createElement =
    (~children as _, ~command: Types.Commandline.t, ~theme: Theme.t, ()) => {
  component(hooks => {
    let (startStr, endStr) =
      getStringParts(command.position, command.content);
    command.show
      ? (
        hooks,
        <View
          style=Style.[
            width(400),
            height(40),
            backgroundColor(theme.editorBackground),
            flexDirection(`Row),
            alignItems(`Center),
            marginBottom(20),
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
              width(2),
              height(cmdFontSize),
              backgroundColor(cmdFontColor),
            ]
          />
          <Text style=cmdTextStyles text=endStr />
        </View>,
      )
      : (hooks, React.listToElement([]));
  });
};
