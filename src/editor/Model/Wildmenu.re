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
};

let create = () => {items: [], selected: 0, show: false};

let empty = [];

let reduce = (s, action) =>
  switch (action) {
  | WildmenuShow(items) => {
   show: true,
   selected: 0,
   items,
  }
  | WildmenuHide => {
   show: false,
   selected: 0,
   items: empty,
  }
  | WildmenuNext => {
    ...s,
    selected: s.selected + 1,
  }
  | WildmenuPrev => {
    ...s,
    selected: s.selected - 1,
  }
  | _ => s
  };
