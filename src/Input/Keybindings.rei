type t;

let empty: t;

type effect =
  | Execute(string)
  | Text(string)
  | Unhandled(EditorInput.KeyPress.t);

let count: t => int;

let keyDown:
  (~context: WhenExpr.ContextKeys.t, ~key: EditorInput.KeyPress.t, t) =>
  (t, list(effect));

let text: (~text: string, t) => (t, list(effect));

let keyUp:
  (~context: WhenExpr.ContextKeys.t, ~key: EditorInput.KeyPress.t, t) =>
  (t, list(effect));

type keybinding = {
  key: string,
  command: string,
  condition: WhenExpr.t,
};

/*
   [of_yojson_with_errors] parses the keybindings,
   and returns the list of successfully parsed keybindings,
   as well as a list of errors for unsuccessfully parsed keybindings.
 */
let of_yojson_with_errors:
  (~default: list(keybinding)=?, Yojson.Safe.t) =>
  result((t, list(string)), string);
