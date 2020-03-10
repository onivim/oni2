open Oni_Core;

[@deriving show]
type t = {
  editorGroupId: int,
  activeEditorId: option(int),
  editors: [@opaque] IntMap.t(Feature_Editor.Editor.t), // TODO: internal
  bufferIdToEditorId: [@opaque] IntMap.t(int), // TODO: internal
  reverseTabOrder: list(int),
  metrics: Feature_Editor.EditorMetrics.t,
};

let create: unit => t;

let getMetrics: t => Feature_Editor.EditorMetrics.t;
let getActiveEditor: t => option(Feature_Editor.Editor.t);
let setActiveEditor: (t, int) => t;
let getEditorById: (int, t) => option(Feature_Editor.Editor.t);
let getOrCreateEditorForBuffer: (t, int) => (t, Feature_Editor.EditorId.t);
let nextEditor: t => t;
let previousEditor: t => t;
let removeEditorById: (t, int) => t;

let isEmpty: t => bool;
