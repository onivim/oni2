//
// InputStateMachine.re
//
// State management for input (keys pressed, potential bindings, etc)

module Input =
  EditorInput.Make({
    type context = WhenExpr.ContextKeys.t;
    type command = string;
  });

type effect =
  Input.effect =
    | Execute(string)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;

type t = Input.t;

let empty = Input.empty;
let count = Input.count;
let keyDown = Input.keyDown;
let text = Input.text;
let keyUp = Input.keyUp;

let addBinding = Input.addBinding;
let addMapping = Input.addMapping;
