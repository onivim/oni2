/*
 * KeyDisplayer.rei
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

type t =
  pri {
    // Enabled is whether or not the key displayer feature is turned on
    isEnabled: bool,
    // Active is whether or not there is a visible key press.
    isActive: bool,
    presses: list(groupedPresses),
  };

let initial: t;

let enable: t => t;

/*
   [update(time, v)] updates the current time of the key displayer.
   This will expire any key presses beyond a few seconds. If there
   are no longer key presses to show, [getActive] will return false.
 */
let update: (float, t) => t;

/*
   [add(time, key, v)] adds a keypress [key]
 */
let add: (float, string, t) => t;

let toString: t => string;
