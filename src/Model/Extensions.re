/*
 * Extensions.re
 *
 * This module models state around loaded / activated extensions
 * for the 'Hover' view
 */

type t = {activatedIds: list(string)};

let empty = {activatedIds: []};

let activateExtension = (id: string, v: t) => {
  activatedIds: [id, ...v.activatedIds],
};

let reduce = (action: Actions.t, v: t) => {
  switch (action) {
  | ExtensionActivated(id) => activateExtension(id, v)
  | _ => v
  };
};
