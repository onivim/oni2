/*
 * GlobalContext.re
 *
 * This is a workaround for the lack of Context API today in brisk-reconciler: https://github.com/briskml/brisk-reconciler/issues/16
 * The idea is, prior to rendering, we'll pull up the stuff we need across the render tree
 * (things that would be provided via `<Provider />`), and store them here.
 *
 * Hopefully, once there is a context API, this can be wholly replaced with it!
 */
open Oni_Core;
open Oni_Model;

type notifyWindowTreeSizeChanged = (~width: int, ~height: int, unit) => unit;
type editorScrollDelta =
  (~editorId: Feature_Editor.EditorId.t, ~deltaY: float, unit) => unit;
type editorSetScroll =
  (~editorId: Feature_Editor.EditorId.t, ~scrollY: float, unit) => unit;

type t = {
  notifyWindowTreeSizeChanged,
  editorScrollDelta,
  editorSetScroll,
  openEditorById: int => unit,
  closeEditorById: int => unit,
  dispatch: Actions.t => unit,
};

let viewNoop: Views.viewOperation =
  (~path as _="", ~id as _=0, ~openMethod as _=Buffer, ()) => ();

let default = {
  notifyWindowTreeSizeChanged: (~width as _, ~height as _, ()) => (),
  editorScrollDelta: (~editorId as _, ~deltaY as _, ()) => (),
  editorSetScroll: (~editorId as _, ~scrollY as _, ()) => (),
  openEditorById: _ => (),
  dispatch: _ => (),
  closeEditorById: _ => (),
};

let _current: ref(t) = ref(default);

let current = () => _current^;

let set = (v: t) => _current := v;
