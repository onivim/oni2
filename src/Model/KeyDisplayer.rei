/*
 * KeyDisplayer.rei
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

type t;

let empty: t;

let setEnabled: (bool, t) => t;

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

let show: t => string;

/*
   [getPresses(v)] returns a list of active presses
 */
let getPresses: t => list(groupedPresses);

/*
   [getEnabled(v)] returns whether or not the key displayer is enabeld
 */
let getEnabled: t => bool;

/*
   [getActive(v)] returns whether or not there are any active key presses.
   An active key press is one that currently should be displayed (ie, hasn't expired).
 */
let getActive: t => bool;
