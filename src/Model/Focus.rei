[@deriving show]
type focusable =
  | Editor
  | Wildmenu
  | Quickmenu
  // Sidebar
  | Extensions
  | FileExplorer
  | SCM
  | Search
  | Pane
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
