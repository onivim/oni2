/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

// We group key presses in time together,
// so they show in the same line
type groupedPresses = {
  isExclusive: bool,
  time: float,
  keys: list(string),
};

type t = {
  // Enabled is whether or not the key displayer feature is turned on
  isEnabled: bool,
  // Active is whether or not there is a visible key press.
  isActive: bool,
  presses: list(groupedPresses),
};

let initial: t = {isActive: false, presses: [], isEnabled: false};

module Constants = {
  let timeToShow = 2.5;
  let timeToGroup = 0.3;
};

let enable = _model => {
  {...initial, isEnabled: true};
};

let update = (time, model) => {
  let presses =
    List.filter(
      press => time -. press.time < Constants.timeToShow,
      model.presses,
    );
  let isActive = List.length(presses) > 0;
  {...model, isActive, presses};
};

let add = (time, key, model) => {
  let isExclusive = String.length(key) > 1 || key == " ";
  let presses =
    switch (model.presses) {
    | [] => [{time, isExclusive, keys: [key]}]
    | [hd, ...tail] =>
      if (time
          -. hd.time <= Constants.timeToGroup
          && !hd.isExclusive
          && String.length(key) == 1) {
        [
          // The key presses was within the group time,
          // so we'll just add it to an existing group
          {time, isExclusive, keys: [key, ...hd.keys]},
          ...tail,
        ];
      } else {
        let isExclusive = String.length(key) > 1;
        [
          // The time was past the group time..
          // so we'll create a new group
          {time, isExclusive, keys: [key]},
          hd,
          ...tail,
        ];
      }
    };

  let ret = {...model, isActive: true, presses};

  // Also filter out old key presses, while we're here
  update(time, ret);
};

let toString = (v: t) => {
  let presses =
    String.concat(
      ",\n",
      List.map(
        (gp: groupedPresses) =>
          Printf.sprintf(
            "{ time: %f keys: [\n%s]}\n",
            gp.time,
            String.concat(",\n", gp.keys),
          ),
        v.presses,
      ),
    );

  Printf.sprintf(
    {|KeyDisplayer: [
 - active: %b
 - enabled: %b
%s
]|},
    v.isActive,
    v.isEnabled,
    presses,
  );
};
