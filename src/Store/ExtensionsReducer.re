/*
 * ExtensionsReducer.re
 */
open Oni_Model;

let reduce = (action: Actions.t, v: Extensions.t) => {
  switch (action) {
  | Extension(Activated(id)) => Extensions.activateExtension(id, v)
  | Extension(Discovered(extensions)) => v
  | _ => v
  };
};
