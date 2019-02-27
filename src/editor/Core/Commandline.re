/*
 * Commandline.re
 */
open Actions;

  [@deriving show]
  type t = {
    content: string,
    firstC: string,
    position: int,
    level: int,
    indent: int,
    prompt: string,
    show: bool,
  };

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
    | CommandlineShow(commandline) => commandline,
    | CommandlineHide(commandline) => commandline,
    | CommandlineUpdate((position, level)) => {
          ...s,
          position,
          level,
      }
    | _ => s
    }
};
