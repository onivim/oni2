/*
 * Commandline.re
 */
open Oni_Core.Types;

open Actions;

[@deriving show]
type t = commandline;

let create = () => {
  content: "",
  firstC: "",
  prompt: "",
  position: 0,
  indent: 0,
  level: 0,
  show: false,
};

let reduce = (s: t, action) => {
  switch (action) {
  | CommandlineShow(commandline) => commandline
  | CommandlineHide(commandline) => commandline
  | CommandlineUpdate((position, level)) => {...s, position, level}
  | _ => s
  };
};
