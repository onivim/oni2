/*
 * GlobalContext.re
 *
 * This is a workaround for the lack of Context API today in brisk-reconciler: https://github.com/briskml/brisk-reconciler/issues/16
 * The idea is, prior to rendering, we'll pull up the stuff we need across the render tree
 * (things that would be provided via `<Provider />`), and store them here.
 *
 * Hopefully, once there is a context API, this can be wholly replaced with it!
 */
open Oni_Core.Types;
open Oni_Model;

type notifySizeChanged = (~width: int, ~height: int, unit) => unit;
type editorScroll = (~deltaY: float, unit) => unit;

type t = {
  notifySizeChanged,
  editorScroll,
  openFileById: int => unit,
  closeFileById: int => unit,
  dispatch: Actions.t => unit,
  state: State.t,
};

let viewNoop: Views.viewOperation =
  (~path as _="", ~id as _=0, ~openMethod as _=Buffer, ()) => ();

let default = {
  state: State.create(),
  notifySizeChanged: (~width as _, ~height as _, ()) => (),
  editorScroll: (~deltaY as _, ()) => (),
  openFileById: _ => (),
  dispatch: _ => (),
  closeFileById: _ => (),
};

let _current: ref(t) = ref(default);

let current = () => _current^;

let set = (v: t) => _current := v;
