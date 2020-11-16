/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

open Oni_Core;

type keyEvent =
  | Key(string)
  | Text(string);

let keyEventToString =
  fun
  | Key(str) => str
  | Text(str) => str;

// MODEL

// We group key presses in time together,
// so they show in the same line
type group = {
  id: float, // use time of first keypress as id
  isExclusive: bool,
  time: float,
  keys: list(keyEvent),
};

type t = {
  isEnabled: bool,
  groups: list(group),
};

type msg =
  | Tick({totalTime: Revery.Time.t});

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

let keysToIgnore = [
  "Left Shift",
  "Right Shift",
  "Right Option",
  "Left Option",
  "Right Command",
  "Left Command",
  "Left Ctrl",
  "Right Ctrl",
];

let shouldIgnore = key => {
  keysToIgnore
  |> List.exists(filterKey =>
       Utility.StringEx.endsWith(~postfix=filterKey, key)
     );
};

let add = (~time, key, model) => {
  let str = keyEventToString(key);
  if (shouldIgnore(str)) {
    model;
  } else {
    let groups =
      switch (key) {
      | Text(text) =>
        switch (model.groups) {
        | [] => [{id: time, time, isExclusive: false, keys: [Text(text)]}]
        | [group, ...rest] =>
          //time = time;
          let keys' =
            switch (group.keys) {
            | [Key(_), ...tail] => [Text(text), ...tail]
            | list => [Text(text), ...list]
            };
          [{...group, keys: keys', time}, ...rest];
        }
      | Key(keyString) =>
        let isCharKey = Zed_utf8.length(keyString) == 1;
        let isExclusive = !isCharKey || keyString == " ";
        let isWithinGroupingInterval = group =>
          time -. group.time <= Constants.maxGroupingInterval;
        let canGroupWith = group =>
          !group.isExclusive && isCharKey && isWithinGroupingInterval(group);

        switch (model.groups) {
        | [] => [{id: time, time, isExclusive, keys: [key]}]

        | [group, ...rest] when canGroupWith(group) =>
          let keys' = [key, ...group.keys];
          [{...group, time, keys: keys'}, ...rest];

        | groups => [
            {id: time, time, isExclusive: !isCharKey, keys: [key]},
            ...groups,
          ]
        };
      };
    {...model, groups} |> removeExpired(time);
  };
};

let keyPress = (~time, key, model) => add(~time, Key(key), model);
let textInput = (~time, text, model) => {
  let text = String.equal(text, " ") ? "Space" : text;
  add(~time, Text(text), model);
};

let update = (msg, model) => {
  switch (msg) {
  | Tick({totalTime}) =>
    removeExpired(Revery.Time.toFloatSeconds(totalTime), model)
  };
};

let sub = model => {
  model.groups
  |> List.map(group => {
       Service_Time.Sub.once(
         ~uniqueId="KeyDisplayer:" ++ string_of_float(group.time),
         ~delay=Revery.Time.ofFloatSeconds(Constants.duration),
         ~msg=(~current) =>
         Tick({totalTime: current})
       )
     })
  |> Isolinear.Sub.batch;
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

  let text = [
    textWrap(TextWrapping.NoWrap),
    Style.backgroundColor(backgroundColor),
    color(Colors.white),
    marginHorizontal(16),
    marginVertical(8),
    flexGrow(0),
  ];
};

let keyGroupView = (~uiFont: UiFont.t, ~text: string, ()) =>
  <View style=Styles.group>
    <Text
      style=Styles.text
      fontFamily={uiFont.family}
      fontSize={uiFont.size}
      text
    />
  </View>;

let make = (~model, ~uiFont, ~top=?, ~left=?, ~right=?, ~bottom=?, ()) => {
  let activeGroups = model.groups;

  let groups =
    List.map(
      group => {
        let text =
          group.keys
          |> List.map(keyEventToString)
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
