//
// InputStateMachine.re
//
// State management for input (keys pressed, potential bindings, etc)

type execute =
  | NamedCommand(string)
  | VimExCommand(string);

module Input =
  EditorInput.Make({
    type context = WhenExpr.ContextKeys.t;
    type command = execute;
  });

include Input;

type effect =
  Input.effect =
    | Execute(execute)
    | Text(string)
    | Unhandled(EditorInput.KeyPress.t)
    | RemapRecursionLimitHit;
