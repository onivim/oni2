open Oni_Core.Types;

let content = (effects: Effects.t) => {
  let dirs = Revery.Environment.getWorkingDirectory() |> Sys.readdir;
  Array.to_list(dirs)
  |> List.map(file =>
       UiMenu.{name: file, command: () => effects.openFile(~path=file, ())}
     );
};
