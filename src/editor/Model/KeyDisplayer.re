/*
 * KeyDisplayer.re
 *
 * State for displaying key-presses in the UI
 */

// We group key presses in time together,
// so they show in the same line
type groupedPresses = {
  time: float,
  keys: list(string),
}

type t = list(groupedPresses);

let empty: t = [];

module Constants {
  let timeToShow = 2.; // two seconds
  let timeToGroup = 0.2; // 200 ms
}

let update = (time, v: t) => {
  let f = (k) => time -. k.time < Constants.timeToShow;
  List.filter(f, v);
};

let add = (time, key, v: t)  => {
  let ret = switch (v) {
  | [] => [{ time, keys: [key] }]
  | [hd, ...tail] => {
    if (time -. hd.time <= Constants.timeToGroup) {
      // The key presses was within the group time,
      // so we'll just add it to an existing group
      [{time: hd.time, keys: [key, ...hd.keys]}, ...tail];
    } else {
      // The time was past the group time..
      // so we'll create a new group
      [{time, keys: [key]}, hd, ...tail];
    }
  }
  }

  // Also filter out old key presses, while we're here
  update(time, ret);
};

let show = (v: t) => {
  "KeyDisplayer: [\n" ++
  String.concat(",\n", List.map((gp: groupedPresses) => {
    "{ time: " ++ string_of_float(gp.time) ++ " keys: [\n" ++
    String.concat(",\n", gp.keys)
    ++ "]}\n"
  }, v))
  ++ "]\n";
};
