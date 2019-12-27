/*
 * ExtensionsReducer.re
 */
open Oni_Model;

let reduce = (action: Actions.t, v: Extensions.t) => {
  switch (action) {
  | Extension(Activated(id)) => Extensions.markActivated(id, v)
  | Extension(Discovered(extensions)) => Extensions.add(extensions, v)
  | _ => v
  };
};
