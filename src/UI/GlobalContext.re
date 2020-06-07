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


type t = {
  dispatch: Actions.t => unit,
};


let default = {
  dispatch: _ => (),
};

let _current: ref(t) = ref(default);

let current = () => _current^;

let set = (v: t) => _current := v;
