/*
 * Wildmenu.re
 *
 */
open Actions;

[@deriving show]
type t = {
  items: list(string),
  show: bool,
  selected: int,
};

let create = () => {items: [], selected: 0, show: false};

let reduce = (s, action) => {
  switch (action) {
  | WildmenuShow(wildmenu) => wildmenu
  | WildmenuHide(wildmenu) => wildmenu
  | WildmenuSelected(selected) => {...s.wildmenu, selected}
  | _ => s
  };
};
