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
  active: bool,
  presses: list(groupedPresses),
};

let empty: t = {active: false, presses: []};

module Constants = {
  let timeToShow = 2.5;
  let timeToGroup = 0.3;
};

let update = (time, v: t) => {
  let f = k => time -. k.time < Constants.timeToShow;
  let presses = List.filter(f, v.presses);
  let active = List.length(presses) > 0;
  {active, presses};
};

let add = (time, key, v: t) =>
  if (!Oni_Input.Filter.filter(key)) {
    v;
  } else {
    let exclusive = String.length(key) > 1;
    let presses =
      switch (v.presses) {
      | [] => [{time, exclusive, keys: [key]}]
      | [hd, ...tail] =>
        if (time -. hd.time <= Constants.timeToGroup && !hd.exclusive && String.length(key) == 1) {
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

    let ret = {active: true, presses};

    // Also filter out old key presses, while we're here
    update(time, ret);
  };

let show = (v: t) => {
  "KeyDisplayer: [\n"
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
