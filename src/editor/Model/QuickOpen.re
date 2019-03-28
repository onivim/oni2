open Oni_Core;
open Types;
open UiMenu;

let stringToCommand = (effects: Effects.t(Actions.t), str) => {
  name: str,
  command: () => effects.openFile(~path=str, ()),
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
          |> List.map(stringToCommand(effects))
          |> (content => effects.dispatch(MenuUpdate(content)) |> ignore)
        );
        [];
      }
    | None => []
  );
