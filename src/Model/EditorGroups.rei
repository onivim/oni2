type t;

let create: unit => t;

let activeGroupId: t => int; // TODO: Should return an option, or be removed entirely
let getEditorGroupById: (t, int) => option(EditorGroup.t);
let getActiveEditorGroup: t => option(EditorGroup.t);

let getFirstEditorGroup: t => EditorGroup.t;

let add: (~defaultFont: Service_Font.font, EditorGroup.t, t) => t;

let closeEditor: (~editorId: int, t) => t;

let isActive: (t, EditorGroup.t) => bool;

let reduce: (~defaultFont: Service_Font.font, t, Actions.t) => t;

let setBufferFont: (~bufferId: int, ~font: Service_Font.font, t) => t;

let setActiveEditor: (~editorId: int, t) => t;
