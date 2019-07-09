/*
 * Wildmenu.re
 *
 */
open Oni_Core.Types;
open Actions;

[@deriving show({with_path: false})]
type t = {
  items: list(string),
  show: bool,
  selected: int,
  count: int,
};

let empty = [];

let create = () => {items: empty, selected: 0, show: false, count: 0};

let getSelectedItem = (v: t) =>
  if (v.count <= 0 || v.selected < 0 || v.selected > v.count - 1) {
    None;
  } else {
    Some(List.nth(v.items, v.selected));
  };

let reduce = (s, action) =>
  switch (action) {
  | WildmenuShow(items) => {
      show: true,
      selected: (-1),
      items,
      count: List.length(items),
    }
  | CommandlineHide
  | WildmenuHide => {show: false, selected: 0, items: empty, count: 0}
  | WildmenuNext => {...s, selected: (s.selected + 1) mod s.count}
  | WildmenuPrevious => {...s, selected: (s.selected - 1) mod s.count}
  | _ => s
  };
