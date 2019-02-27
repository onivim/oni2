/*
 * Wildmenu.re
 *
 */
open Actions;
open Types;

[@deriving show]
type t = wildmenu;

let create = () => {items: [], selected: 0, show: false};

let reduce = (s, action) => {
  switch (action) {
  | WildmenuShow(wildmenu) => wildmenu
  | WildmenuHide(wildmenu) => wildmenu
  | WildmenuSelected(selected) => {...s, selected}
  | _ => s
  };
};
