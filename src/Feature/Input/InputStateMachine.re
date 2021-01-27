//
// InputStateMachine.re
//
// State management for input (keys pressed, potential bindings, etc)

type execute =
  | NamedCommand({
      command: string,
      arguments: Yojson.Safe.t,
    })
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
    | Unhandled({
        key: EditorInput.KeyCandidate.t,
        isProducedByRemap: bool,
      })
    | RemapRecursionLimitHit;
