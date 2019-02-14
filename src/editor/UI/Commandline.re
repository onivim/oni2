open Revery.UI;
open Revery.Core;
open Oni_Core;

let component = React.component("commandline");

let cmdFontSize = 20;
let cmdFontColor = Colors.white;

let cmdTextStyles =
  Style.[
    fontFamily("FiraCode-Regular.ttf"),
    fontSize(cmdFontSize),
    color(cmdFontColor),
    textWrap(TextWrapping.WhitespaceWrap),
    marginVertical(8),
  ];

/*
   TODO: Flow text around a "cursor"
 */

let getStringParts = (index, str) => {
  switch (index) {
  | 0 => ("", str)
  | _ =>
    let strBeginning = Str.string_before(str, index);
    let strEnd = Str.string_after(str, index);
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
            overflow(LayoutTypes.Hidden),
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
          <Text
            style=Style.[marginLeft(10), ...cmdTextStyles]
            text={command.firstC ++ startStr}
          />
          <View
            style=Style.[
              width(2),
              alignSelf(`FlexEnd),
              height(cmdFontSize),
              backgroundColor(cmdFontColor),
              marginVertical(8),
            ]
          />
          <Text style=cmdTextStyles text=endStr />
        </View>,
      )
      : (hooks, React.listToElement([]));
  });
};
