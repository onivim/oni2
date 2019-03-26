open Oni_Core.Types;
open UiMenu;

let content = (effects: Effects.t) =>
  effects.getCurrentDir()
  |> (
    fun
    | Some(dir) => Sys.readdir(dir)
    | None => [||]
  )
  |> Array.to_list
  /*
     In the future we might want to allow
     functionality like switching to a directory on select
     ...for now we filter out all directories
   */
  |> List.filter(item => !Sys.is_directory(item))
  |> List.map(file =>
       {
         name: file,
         command: () => effects.openFile(~path=file, ()),
         icon: Some({||}),
       }
     );
