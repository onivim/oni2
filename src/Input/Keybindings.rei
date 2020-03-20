type t;

let empty: t;

type effect =
| Command(string)
| Unhandled(EditorInput.key);

let count: t => int;

let keyDown: (
  ~context: Hashtbl.t(string, bool), 
  ~key: EditorInput.key,
  t) => (t, list(effect));

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
  (~default:list(keybinding)=?, Yojson.Safe.t) => result((t, list(string)), string);
