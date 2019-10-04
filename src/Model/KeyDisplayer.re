/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

// We group key presses in time together,
// so they show in the same line
type groupedPresses = {
  exclusive: bool,
  time: float,
  keys: list(string),
};

type t = {
  // Enabled is whether or not the key displayer feature is turned on
  enabled: bool,
  // Active is whether or not there is a visible key press.
  active: bool,
  presses: list(groupedPresses),
};

let empty: t = {active: false, presses: [], enabled: false};

module Constants = {
  let timeToShow = 2.5;
  let timeToGroup = 0.3;
};

let setEnabled = (enabled, _v: t) => {
  {...empty, enabled};
};

let update = (time, v: t) => {
  let f = k => time -. k.time < Constants.timeToShow;
  let presses = List.filter(f, v.presses);
  let active = List.length(presses) > 0;
  {...v, active, presses};
};

let add = (time, key, v: t) => {
  let exclusive = String.length(key) > 1 || key == " ";
  let presses =
    switch (v.presses) {
    | [] => [{time, exclusive, keys: [key]}]
    | [hd, ...tail] =>
      if (time
          -. hd.time <= Constants.timeToGroup
          && !hd.exclusive
          && String.length(key) == 1) {
        [
          // The key presses was within the group time,
          // so we'll just add it to an existing group
          {time, exclusive, keys: [key, ...hd.keys]},
          ...tail,
        ];
      } else {
        let exclusive = String.length(key) > 1;
        [
          // The time was past the group time..
          // so we'll create a new group
          {time, exclusive, keys: [key]},
          hd,
          ...tail,
        ];
      }
    };

  let ret = {...v, active: true, presses};

  // Also filter out old key presses, while we're here
  update(time, ret);
};

let getPresses = (v: t) => v.presses;

let getEnabled = (v: t) => v.enabled;

let getActive = (v: t) => v.enabled && v.active;

let show = (v: t) => {
  "KeyDisplayer: [\n"
  ++ " - active: " ++ (v.active ? "true": "false") ++ "\n"
  ++ " - enabled: " ++ (v.enabled ? "true": "false") ++ "\n"
  ++ String.concat(
       ",\n",
       List.map(
         (gp: groupedPresses) =>
           "{ time: "
           ++ string_of_float(gp.time)
           ++ " keys: [\n"
           ++ String.concat(",\n", gp.keys)
           ++ "]}\n",
         v.presses,
       ),
     )
  ++ "]\n";
};
