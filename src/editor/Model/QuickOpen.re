open Oni_Core;
open Types;
open UiMenu;

/**
   remove the parent directory from the path rendered since searches
   are relative to the directory passed in and the displayed string
   is too long otherwise
 */
let getDisplayPath = (fullPath, dir) => {
  let re = Str.regexp_string(dir ++ Filename.dir_sep);
  Str.replace_first(re, "", fullPath);
};

let stringToCommand = (effects: Effects.t(Actions.t), parentDir, fullPath) => {
  name: getDisplayPath(fullPath, parentDir),
  command: () => effects.openFile(~path=fullPath, ()),
  icon: Some({||}),
};

let content = (effects: Effects.t(Actions.t)) =>
  effects.getCurrentDir()
  |> (
    fun
    | Some(dir) => {
        effects.ripgrep.search(dir, items =>
          items
          |> List.filter(item => !Sys.is_directory(item))
          |> List.map(stringToCommand(effects, dir))
          |> (content => effects.dispatch(MenuUpdate((QuickOpen, content))))
          |> ignore
        );
        [];
      }
    | None => []
  );
