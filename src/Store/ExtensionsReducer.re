/*
 * ExtensionsReducer.re
 */
open Oni_Model;

let reduce = (action: Actions.t, v: Extensions.t) => {
  switch (action) {
  | Extension(Activated(id)) => Extensions.notifyActivated(id, v)
  | Extension(Discovered(extensions)) =>
    Extensions.notifyDiscovered(extensions, v)
  | _ => v
  };
};
