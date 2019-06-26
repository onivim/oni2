/*
 * Commandline.re
 */
open Vim.Types;
open Oni_Core.Types;

open Actions;

type t = commandline;

let default = {text: "", cmdType: Ex, position: 0, show: false};

let create = () => {text: "", cmdType: Ex, position: 0, show: false};

let reduce = (s: t, action) => {
  switch (action) {
  | CommandlineShow(cmdType) => {...default, show: true, cmdType}
  | CommandlineHide => default
  | CommandlineUpdate({cmdType, position, text}) => {
      ...s,
      position,
      cmdType,
      text,
    }
  | _ => s
  };
};
