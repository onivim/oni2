[@deriving show({with_path: false})]
type focusable =
  | Editor // TODO: Carry editor/buffer id
  | Wildmenu
  | Quickmenu
  | Search
  | FileExplorer;

type stack = list(focusable);

let initial = [];

let push = focusable =>
  fun
  | [head, ..._] as stack when head == focusable => stack
  | stack => [focusable, ...stack];

let pop = focusable =>
  fun
  | [head, ...rest] when head == focusable => rest
  | stack => stack;

let current =
  fun
  | [head, ..._] => Some(head)
  | _ => None;
