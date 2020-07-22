[@deriving show]
type focusable =
  | Editor
  | Extensions
  | Wildmenu
  | Quickmenu
  | Search
  | FileExplorer
  | SCM
  | Sneak
  | Modal
  | InsertRegister
  | LanguageSupport
  | Terminal(int);

type stack;

let initial: stack;

let push: (focusable, stack) => stack;
let pop: (focusable, stack) => stack;

let current: stack => option(focusable);
