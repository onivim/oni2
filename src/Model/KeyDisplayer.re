/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

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
  let timeToShow = 2.5;
  let maxGroupInterval = 0.3;
};

let enable = _model => {
  {...initial, isEnabled: true};
};

let remvoeExpired = (time, model) => {
  let groups =
    List.filter(
      press => time -. press.time < Constants.timeToShow,
      model.groups,
    );
  {...model, groups};
};

let add = (time, key, model) => {
  let isCharKey = String.length(key) == 1;
  let isExclusive = !isCharKey || key == " ";
  let isWithinGroupInterval = group =>
    time -. group.time <= Constants.maxGroupInterval;
  let canGroupWith = group =>
    !group.isExclusive && isCharKey && isWithinGroupInterval(group);

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

  {...model, groups} |> remvoeExpired(time);
};

let toString = model => {
  let groups =
    String.concat(
      ",\n",
      List.map(
        (group: group) =>
          Printf.sprintf(
            "{ time: %f keys: [\n%s]}\n",
            group.time,
            String.concat(",\n", group.keys),
          ),
        model.groups,
      ),
    );

  Printf.sprintf(
    {|KeyDisplayer: [
 - active: %b
 - enabled: %b
%s
]|},
    model.groups != [],
    model.isEnabled,
    groups,
  );
};
