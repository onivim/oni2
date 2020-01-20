open Oni_Core;
open Oni_Core_Kernel;

type t =
  Actions.editorGroup = {
    editorGroupId: int,
    activeEditorId: option(int),
    editors: IntMap.t(Editor.t), // TODO: internal
    bufferIdToEditorId: IntMap.t(int), // TODO: internal
    reverseTabOrder: list(int),
    metrics: EditorMetrics.t,
  };

let create: unit => t;

let getMetrics: t => EditorMetrics.t;
let getActiveEditor: t => option(Editor.t);
let setActiveEditor: (t, int) => t;
let getEditorById: (int, t) => option(Editor.t);
let getOrCreateEditorForBuffer: (t, int) => (t, EditorId.t);
let removeEditorById: (t, int) => t;

let isEmpty: t => bool;
