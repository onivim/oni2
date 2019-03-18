open Oni_Core.Types;

let content = (effects: Effects.t) =>
  effects.getCurrentDir()
  |> (
    fun
    | Some(dir) => Sys.readdir(dir)
    | None => [||]
  )
  |> Array.to_list
  |> List.map(file =>
       UiMenu.{name: file, command: () => effects.openFile(~path=file, ())}
     );
