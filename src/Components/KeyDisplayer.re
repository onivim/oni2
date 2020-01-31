/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

open Oni_Core;

module List = Utility.List;

// MODEL

// We group key presses in time together,
// so they show in the same line
type group = {
  id: float, // use time of first keypress as id
  isExclusive: bool,
  mutable time: float,
  mutable keys: list(string),
};

type t = {
  isEnabled: bool,
  groups: list(group),
};

let initial: t = {groups: [], isEnabled: false};

module Constants = {
  let duration = 2.5;
  let maxGroupingInterval = 0.3;
};

// UPDATE-ish

let removeExpired = (time, model) => {
  let groups =
    List.filter(
      press => time -. press.time < Constants.duration,
      model.groups,
    );
  {...model, groups};
};

let add = (~time, key, model) => {
  let isCharKey = String.length(key) == 1;
  let isExclusive = !isCharKey || key == " ";
  let isWithinGroupingInterval = group =>
    time -. group.time <= Constants.maxGroupingInterval;
  let canGroupWith = group =>
    !group.isExclusive && isCharKey && isWithinGroupingInterval(group);

  let groups =
    switch (model.groups) {
    | [] => [{id: time, time, isExclusive, keys: [key]}]

    | [group, ..._] as groups when canGroupWith(group) =>
      group.time = time;
      group.keys = [key, ...group.keys];
      groups;

    | groups => [
        {id: time, time, isExclusive: !isCharKey, keys: [key]},
        ...groups,
      ]
    };

  {...model, groups} |> removeExpired(time);
};

// VIEW

open Revery;
open Revery.UI;
open Revery.UI.Components;

module Styles = {
  open Style;

  let backgroundColor = Color.rgb(0.1, 0.1, 0.1);

  let group = [
    Style.backgroundColor(backgroundColor),
    justifyContent(`Center),
    alignItems(`Center),
    margin(8),
    flexGrow(0),
  ];

  let text = (uiFont: UiFont.t) => [
    fontFamily(uiFont.fontFile),
    fontSize(24.),
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

let%component make =
              (~model, ~uiFont, ~top=?, ~left=?, ~right=?, ~bottom=?, ()) => {
  let%hook activeGroups =
    CustomHooks.useExpiration(
      ~equals=(a, b) => a.id == b.id,
      ~expireAfter=Time.ms(int_of_float(Constants.duration *. 1000.)),
      model.groups,
    );

  let groups =
    List.map(
      group => {
        let text =
          group.keys
          |> List.map(Oni_Input.Parser.toFriendlyName)
          |> List.rev
          |> String.concat("");
        <keyGroupView uiFont text />;
      },
      activeGroups,
    )
    |> List.rev
    |> React.listToElement;

  <Positioned ?top ?left ?bottom ?right> groups </Positioned>;
};
