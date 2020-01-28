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

module List = Core.Utility.List;

module Constants = {
  let margin = 50;
  let duration = Time.ms(2500);
};

module Styles = {
  open Style;

  let backgroundColor = Color.rgb(0.1, 0.1, 0.1);
  let foregroundColor = Color.rgb(0.9, 0.9, 0.9);

  let group = [
    Style.backgroundColor(backgroundColor),
    justifyContent(`Center),
    alignItems(`Center),
    margin(8),
    flexGrow(0),
  ];

  let text = uiFont => [
    fontFamily(uiFont),
    fontSize(24),
    textWrap(TextWrapping.NoWrap),
    Style.backgroundColor(backgroundColor),
    color(Colors.white),
    marginHorizontal(16),
    marginVertical(8),
    flexGrow(0),
  ];
};

let keyGroupView = (~uiFont, ~text: string, ()) =>
  <View style=Styles.group> <Text style={Styles.text(uiFont)} text /> </View>;

let%component make = (~state: State.t, ()) => {
  let%hook activePresses =
    CustomHooks.useExpiration(
      ~expireAfter=Constants.duration,
      state.keyDisplayer.presses,
    );
  let uiFont = state.uiFont.fontFile;

  let groups =
    List.map(
      (keyGroup: KeyDisplayer.groupedPresses) => {
        let text =
          keyGroup.keys
          |> List.map(Oni_Input.Parser.toFriendlyName)
          |> List.rev
          |> String.concat("");
        <keyGroupView uiFont text />;
      },
      activePresses,
    )
    |> List.rev
    |> React.listToElement;

  let bottom = Constants.margin;
  let right =
    Notifications.any(state.notifications)
      ? Core.Constants.default.notificationWidth + Constants.margin * 2
      : Constants.margin;

  <Positioned bottom right> groups </Positioned>;
};
