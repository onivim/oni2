/*
 * Input.rei
 */

open Revery_Core;

type t;

let create: (unit) => t;
let show: t => string;

let keyPressToString: (~altKey: bool, ~shiftKey: bool, ~ctrlKey: bool, ~superKey: bool, string) => string;

let keyDown: (t, Events.keyEvent) => (option(string), t);
let keyUp: (t, Events.keyEvent) => (option(string), t);
let keyPress: (t, Events.keyPressEvent) => (option(string), t);
