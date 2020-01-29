/*
 * KeyDisplayer.rei
 *
 * State for displaying key-presses in the UI
 */

// We group key presses in time together,
// so they show in the same line
type group = {
  id: float,
  isExclusive: bool,
  mutable time: float,
  mutable keys: list(string),
};

type t =
  pri {
    isEnabled: bool,
    groups: list(group),
  };

let initial: t;

let enable: t => t;

/*
   [add(time, key, v)] adds a keypress [key]
 */
let add: (float, string, t) => t;

let toString: t => string;
