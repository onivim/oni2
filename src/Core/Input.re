[@deriving (show({with_path: false}), yojson({strict: false, exn: false}))]
// This type is overloaded - describing both the current 'input mode'
// the UI is in, as well as the state of 'when' conditions in the input
// bindings. Need to decouple these.
type controlMode =
  // VSCode-compatible when parameters
  | [@name "listFocus"] ListFocus
  | [@name "textInputFocus"] TextInputFocus
  | [@name "inQuickOpen"] InQuickOpen
  | [@name "editorTextFocus"] EditorTextFocus
  | [@name "suggestWidgetVisible"] SuggestWidgetVisible
  // Onivim extensions to the 'when' syntax
  | [@name "insertMode"] InsertMode
  | [@name "visualMode"] VisualMode
  | [@name "quickmenuCursorEnd"] QuickmenuCursorEnd;

[@deriving show({with_path: false})]
type keyBindings = {
  key: string,
  command: string,
};
