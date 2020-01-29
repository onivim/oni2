/*
 * KeyDisplayerView.re
 *
 * View for KeyDisplayer
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Core = Oni_Core;
open Oni_Model;

let bgc = Color.rgb(0.1, 0.1, 0.1);
let fgc = Color.rgb(0.9, 0.9, 0.9);

let containerStyle =
  Style.[
    backgroundColor(bgc),
    justifyContent(`Center),
    alignItems(`Center),
    margin(8),
    flexGrow(0),
  ];

let textStyle = uiFont => {
  Style.[
    fontFamily(uiFont),
    fontSize(24.),
    textWrap(TextWrapping.NoWrap),
    backgroundColor(bgc),
    color(fgc),
    marginHorizontal(16),
    marginVertical(8),
    flexGrow(0),
  ];
};

let keyGroupView = (~uiFont, ~text: string, ()) => {
  <View
    style=Style.[
      backgroundColor(bgc),
      justifyContent(`Center),
      alignItems(`Center),
      margin(8),
      flexGrow(0),
    ]>
    <Text
      style=Style.[
        fontFamily(uiFont),
        fontSize(24.),
        textWrap(TextWrapping.NoWrap),
        backgroundColor(bgc),
        color(Colors.white),
        marginHorizontal(16),
        marginVertical(8),
        flexGrow(0),
      ]
      text
    />
  </View>;
};

let make = (~state: State.t, ()) => {
  let uiFont = state.uiFont.fontFile;

  let anyNotifications = Notifications.any(state.notifications);

  let extraSpacing =
    anyNotifications ? Core.Constants.default.notificationWidth + 50 : 0;

  let groups =
    List.map(
      (keyGroup: KeyDisplayer.groupedPresses) => {
        let f = s =>
          switch (Oni_Input.Parser.toFriendlyName(s)) {
          | None => ""
          | Some(v) => v
          };
        let text =
          String.concat("", keyGroup.keys |> List.map(f) |> List.rev);
        <keyGroupView uiFont text />;
      },
      state.keyDisplayer |> KeyDisplayer.getPresses,
    )
    |> List.rev
    |> React.listToElement;

  <Positioned bottom=50 right={50 + extraSpacing}> groups </Positioned>;
};
