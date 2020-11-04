//
// InputStateMachine.re
//
// State management for input (keys pressed, potential bindings, etc)

module Input =
  EditorInput.Make({
    type context = WhenExpr.ContextKeys.t;
    type command = string;
  });

include Input;

type effect =
  Input.effect =
    | Execute(string)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;
