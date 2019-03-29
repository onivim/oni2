/*
 * StatusBar
 *
 * Model for status bar items
 */

module Alignment = {
  type t =
    | Left
    | Right;

  let ofInt = i =>
    switch (i) {
    | 0 => Left
    | _ => Right
    };
};

module Item = {
  type t = {
    id: int,
    priority: int,
    text: string,
    alignment: Alignment.t,
  };

  let create = (~id, ~priority, ~text, ~alignment=Alignment.Left, ()) => {
    id,
    priority,
    text,
    alignment,
  };
};

type t = list(Item.t);

let create: unit => t =
  () => {
    let item = Item.create(~id=1, ~priority=1000, ~text="HELLO WORLD", ());
    [item];
  };
