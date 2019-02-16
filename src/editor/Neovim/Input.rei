/*
 * Input.rei
 */

open Revery_Core;

type t = string;

let keyPressToString: (~altKey: bool, ~shiftKey: bool, ~ctrlKey: bool, ~superKey: bool, string) => t;

let ofKeyEvent: Events.keyEvent => option(t);

let ofKeyPressEvent: Events.keyPressEvent => t;
